/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

#include "qnemu/gb/gpu/PpuRenderer.h"
#include "qnemu/gb/sgb/GbSgb.h"

namespace qnemu
{

uint32_t PpuRenderer::dmgPixelColor(const PpuMemory& memory,
    int x,
    int y,
    uint8_t colorIndex,
    uint8_t palette) const
{
    if (sgb_) {
        const uint8_t shade = static_cast<uint8_t>((palette >> (colorIndex * 2)) & 3);
        return sgb_->colorForPixel(x, y, shade);
    }
    if (memory.dmgCompatibility()) {
        return memory.dmgCompatibilityColor(false, 0, palette, colorIndex);
    }
    return memory.dmgColor(palette, colorIndex);
}

uint16_t PpuRenderer::tileDataAddress(const PpuRegisters& registers, uint8_t tileIndex)
{
    if (registers.tileDataHigh()) {
        return static_cast<uint16_t>(tileIndex * 16);
    }
    return static_cast<uint16_t>(0x1000 + static_cast<int8_t>(tileIndex) * 16);
}

PpuRenderer::BackgroundPixel PpuRenderer::fetchBackgroundPixel(const PpuRegisters& registers,
    const PpuMemory& memory,
    uint8_t x,
    uint8_t y,
    bool window) const
{
    const bool highMap = window ? registers.windowMapHigh() : registers.bgMapHigh();
    const uint16_t tileMapOffset = static_cast<uint16_t>((highMap ? 0x1C00 : 0x1800)
        + (y / 8) * 32 + (x / 8));
    const uint8_t tileIndex = memory.readVramByte(0, tileMapOffset);
    const uint8_t attributes = cgbRendering() ? memory.readVramByte(1, tileMapOffset) : 0x00;

    BackgroundPixel pixel;
    if (cgbRendering()) {
        pixel.palette = static_cast<uint8_t>(attributes & 0x07);
        pixel.priority = (attributes & 0x80) != 0;
    }

    const uint8_t bank = cgbRendering() && (attributes & 0x08) != 0 ? 1 : 0;
    const uint8_t pixelX = static_cast<uint8_t>((attributes & 0x20) != 0 ? 7 - (x & 7) : (x & 7));
    const uint8_t pixelY = static_cast<uint8_t>((attributes & 0x40) != 0 ? 7 - (y & 7) : (y & 7));
    const uint16_t byteIndex = static_cast<uint16_t>(tileDataAddress(registers, tileIndex) + pixelY * 2);
    const uint8_t bit = static_cast<uint8_t>(7 - pixelX);
    const uint8_t low = memory.readVramByte(bank, byteIndex);
    const uint8_t high = memory.readVramByte(bank, static_cast<uint16_t>(byteIndex + 1));
    pixel.colorIndex = static_cast<uint8_t>(((low >> bit) & 0x01) | (((high >> bit) & 0x01) << 1));
    return pixel;
}

bool PpuRenderer::renderLine(const PpuRegisters& registers,
    const PpuMemory& memory,
    uint8_t line,
    uint16_t windowLine,
    FrameBuffer& frame) const
{
    std::array<uint8_t, FrameBuffer::width> backgroundColorIndex{};
    std::array<bool, FrameBuffer::width> backgroundPriority{};
    bool windowVisible = false;

    const bool backgroundVisible = cgbRendering() || registers.bgWindowEnabled();
    if (!cgbRendering() && sgb_) {
        for (int x = 0; x < FrameBuffer::width; ++x) {
            frame.at(x, line) = dmgPixelColor(memory, x, line, 0, registers.bgp);
        }
    } else {
        const uint32_t backgroundOffColor = !cgbRendering() && memory.dmgCompatibility()
            ? memory.dmgCompatibilityColor(false, 0, registers.bgp, 0)
            : memory.dmgColor(registers.bgp, 0);
        for (int x = 0; x < FrameBuffer::width; ++x) {
            frame.at(x, line) = backgroundOffColor;
        }
    }

    if (backgroundVisible) {
        for (int x = 0; x < FrameBuffer::width; ++x) {
            const uint8_t backgroundX = static_cast<uint8_t>((x + registers.scx) & 0xFF);
            const uint8_t backgroundY = static_cast<uint8_t>((line + registers.scy) & 0xFF);
            const BackgroundPixel pixel = fetchBackgroundPixel(
                registers, memory, backgroundX, backgroundY, false);
            backgroundColorIndex.at(x) = pixel.colorIndex;
            backgroundPriority.at(x) = pixel.priority;
            if (cgbRendering()) {
                frame.at(x, line) = memory.cgbColor(false, pixel.palette, pixel.colorIndex);
            } else {
                frame.at(x, line) = dmgPixelColor(
                    memory, x, line, pixel.colorIndex, registers.bgp);
            }
        }

        if (registers.windowEnabled() && registers.wx >= 7 && registers.wx <= 166
            && line >= registers.wy) {
            const int startX = registers.wx - 7;
            for (int x = startX; x < FrameBuffer::width; ++x) {
                const uint8_t windowX = static_cast<uint8_t>(x - startX);
                const uint8_t windowY = static_cast<uint8_t>(windowLine & 0xFF);
                const BackgroundPixel pixel = fetchBackgroundPixel(
                    registers, memory, windowX, windowY, true);
                backgroundColorIndex.at(x) = pixel.colorIndex;
                backgroundPriority.at(x) = pixel.priority;
                if (cgbRendering()) {
                    frame.at(x, line) = memory.cgbColor(false, pixel.palette, pixel.colorIndex);
                } else {
                    frame.at(x, line) = dmgPixelColor(
                        memory, x, line, pixel.colorIndex, registers.bgp);
                }
                windowVisible = true;
            }
        }
    }

    if (registers.objEnabled()) {
        renderSprites(registers, memory, line, backgroundColorIndex, backgroundPriority, frame);
    }

    return windowVisible;
}

void PpuRenderer::renderSprites(const PpuRegisters& registers,
    const PpuMemory& memory,
    uint8_t line,
    const std::array<uint8_t, FrameBuffer::width>& backgroundColorIndex,
    const std::array<bool, FrameBuffer::width>& backgroundPriority,
    FrameBuffer& frame) const
{
    const int spriteHeight = registers.objSize16() ? 16 : 8;
    std::array<Sprite, 10> sprites{};
    std::size_t spriteCount = 0;

    for (uint8_t index = 0; index < OamSize && spriteCount < sprites.size(); index += 4) {
        const int y = static_cast<int>(memory.readOamByte(index)) - 16;
        if (static_cast<int>(line) < y || static_cast<int>(line) >= y + spriteHeight) {
            continue;
        }
        sprites.at(spriteCount) = Sprite {
            index,
            static_cast<int>(memory.readOamByte(static_cast<uint8_t>(index + 1))) - 8,
            y,
        };
        ++spriteCount;
    }

    const bool xPriority = !cgbRendering() || (registers.opri & 0x01) != 0;
    if (xPriority) {
        std::stable_sort(sprites.begin(), sprites.begin() + static_cast<std::ptrdiff_t>(spriteCount),
            [](const Sprite& left, const Sprite& right) {
                if (left.x != right.x) {
                    return left.x < right.x;
                }
                return left.oamIndex < right.oamIndex;
            });
    }

    std::array<bool, FrameBuffer::width> spritePixel{};
    for (std::size_t spriteIndex = 0; spriteIndex < spriteCount; ++spriteIndex) {
        const Sprite& sprite = sprites.at(spriteIndex);
        const uint8_t tile = memory.readOamByte(static_cast<uint8_t>(sprite.oamIndex + 2));
        const uint8_t attributes = memory.readOamByte(static_cast<uint8_t>(sprite.oamIndex + 3));
        const uint8_t bank = cgbRendering() && (attributes & 0x08) != 0 ? 1 : 0;

        int row = static_cast<int>(line) - sprite.y;
        if ((attributes & 0x40) != 0) {
            row = spriteHeight - 1 - row;
        }

        uint8_t realTile = tile;
        if (spriteHeight == 16) {
            realTile = static_cast<uint8_t>(tile & 0xFE);
            if (row >= 8) {
                realTile = static_cast<uint8_t>(realTile + 1);
            }
        }
        const uint8_t tileRow = static_cast<uint8_t>(row & 0x07);

        for (uint8_t pixel = 0; pixel < 8; ++pixel) {
            const int x = sprite.x + pixel;
            if (x < 0 || x >= FrameBuffer::width || spritePixel.at(x)) {
                continue;
            }
            const uint8_t bit = static_cast<uint8_t>(
                (attributes & 0x20) != 0 ? pixel : 7 - pixel);
            const uint16_t byteIndex = static_cast<uint16_t>(realTile * 16 + tileRow * 2);
            const uint8_t low = memory.readVramByte(bank, byteIndex);
            const uint8_t high = memory.readVramByte(bank, static_cast<uint16_t>(byteIndex + 1));
            const uint8_t colorIndex = static_cast<uint8_t>(
                ((low >> bit) & 0x01) | (((high >> bit) & 0x01) << 1));
            if (colorIndex == 0) {
                continue;
            }

            if (backgroundColorIndex.at(x) != 0) {
                const bool objectPriority = (attributes & 0x80) != 0;
                const bool backgroundToObjectPriority = cgbRendering()
                    && (registers.lcdc & LcdcBits::bgWindowEnable) != 0
                    && backgroundPriority.at(x);
                if (objectPriority || backgroundToObjectPriority) {
                    continue;
                }
            }

            if (cgbRendering()) {
                frame.at(x, line) = memory.cgbColor(
                    true, static_cast<uint8_t>(attributes & 0x07), colorIndex);
            } else {
                const bool objectPalette1 = (attributes & 0x10) != 0;
                const uint8_t palette = objectPalette1 ? registers.obp1 : registers.obp0;
                if (sgb_) {
                    frame.at(x, line) = dmgPixelColor(memory, x, line, colorIndex, palette);
                } else if (memory.dmgCompatibility()) {
                    frame.at(x, line) = memory.dmgCompatibilityColor(
                        true, objectPalette1 ? 1 : 0, palette, colorIndex);
                } else {
                    frame.at(x, line) = memory.dmgColor(palette, colorIndex);
                }
            }
            spritePixel.at(x) = true;
        }
    }
}

}  // namespace qnemu
