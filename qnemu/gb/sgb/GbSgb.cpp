/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/display/SgbFrameBuffer.h"
#include "qnemu/gb/sgb/GbSgb.h"
#include "qnemu/gb/sgb/SgbBorderData.h"

namespace qnemu
{

namespace
{

constexpr uint8_t CommandPAL01 = 0x00;
constexpr uint8_t CommandPAL23 = 0x01;
constexpr uint8_t CommandPAL03 = 0x02;
constexpr uint8_t CommandPAL12 = 0x03;
constexpr uint8_t CommandATTRBLK = 0x04;
constexpr uint8_t CommandATTRLIN = 0x05;
constexpr uint8_t CommandATTRDIV = 0x06;
constexpr uint8_t CommandATTRCHR = 0x07;
constexpr uint8_t CommandPALSET = 0x0A;
constexpr uint8_t CommandPALTRN = 0x0B;
constexpr uint8_t CommandDATASND = 0x0F;
constexpr uint8_t CommandMLTREQ = 0x11;
constexpr uint8_t CommandCHRTRN = 0x13;
constexpr uint8_t CommandPCTTRN = 0x14;
constexpr uint8_t CommandATTRTRN = 0x15;
constexpr uint8_t CommandATTRSET = 0x16;
constexpr uint8_t CommandMASKEN = 0x17;

enum TransferDestination : uint8_t
{
    TransferLowTiles,
    TransferHighTiles,
    TransferPalettes,
    TransferBorderData,
    TransferAttributes,
};

// SGB boot-ROM palette groups.  Values are little-endian RGB555, matching
// SameBoy's MIT-licensed SGB tables.
constexpr std::array<uint16_t, 32 * 4> BuiltInPalettes = {
    0x67BF, 0x265B, 0x10B5, 0x2866,
    0x637B, 0x3AD9, 0x0956, 0x0000,
    0x7F1F, 0x2A7D, 0x30F3, 0x4CE7,
    0x57FF, 0x2618, 0x001F, 0x006A,
    0x5B7F, 0x3F0F, 0x222D, 0x10EB,
    0x7FBB, 0x2A3C, 0x0015, 0x0900,
    0x2800, 0x7680, 0x01EF, 0x2FFF,
    0x73BF, 0x46FF, 0x0110, 0x0066,
    0x533E, 0x2638, 0x01E5, 0x0000,
    0x7FFF, 0x2BBF, 0x00DF, 0x2C0A,
    0x7F1F, 0x463D, 0x74CF, 0x4CA5,
    0x53FF, 0x03E0, 0x00DF, 0x2800,
    0x433F, 0x72D2, 0x3045, 0x0822,
    0x7FFA, 0x2A5F, 0x0014, 0x0003,
    0x1EED, 0x215C, 0x42FC, 0x0060,
    0x7FFF, 0x5EF7, 0x39CE, 0x0000,
    0x4F5F, 0x630E, 0x159F, 0x3126,
    0x637B, 0x121C, 0x0140, 0x0840,
    0x66BC, 0x3FFF, 0x7EE0, 0x2C84,
    0x5FFE, 0x3EBC, 0x0321, 0x0000,
    0x63FF, 0x36DC, 0x11F6, 0x392A,
    0x65EF, 0x7DBF, 0x035F, 0x2108,
    0x2B6C, 0x7FFF, 0x1CD9, 0x0007,
    0x53FC, 0x1F2F, 0x0E29, 0x0061,
    0x36BE, 0x7EAF, 0x681A, 0x3C00,
    0x7BBE, 0x329D, 0x1DE8, 0x0423,
    0x739F, 0x6A9B, 0x7293, 0x0001,
    0x5FFF, 0x6732, 0x3DA9, 0x2481,
    0x577F, 0x3EBC, 0x456F, 0x1880,
    0x6B57, 0x6E1B, 0x5010, 0x0007,
    0x0F96, 0x2C97, 0x0045, 0x3200,
    0x67FF, 0x2F17, 0x2230, 0x1548,
};

struct PaletteAssignment
{
    const char* title;
    uint8_t palette;
};

constexpr PaletteAssignment PaletteAssignments[] = {
    {"ZELDA", 5},
    {"SUPER MARIOLAND", 6},
    {"MARIOLAND2", 0x14},
    {"SUPERMARIOLAND3", 2},
    {"KIRBY DREAM LAND", 0xB},
    {"HOSHINOKA-BI", 0xB},
    {"KIRBY'S PINBALL", 3},
    {"YOSSY NO TAMAGO", 0xC},
    {"MARIO & YOSHI", 0xC},
    {"YOSSY NO COOKIE", 4},
    {"YOSHI'S COOKIE", 4},
    {"DR.MARIO", 0x12},
    {"TETRIS", 0x11},
    {"YAKUMAN", 0x13},
    {"METROID2", 0x1F},
    {"KAERUNOTAMENI", 9},
    {"GOLF", 0x18},
    {"ALLEY WAY", 0x16},
    {"BASEBALL", 0xF},
    {"TENNIS", 0x17},
    {"F1RACE", 0x1E},
    {"KID ICARUS", 0xE},
    {"QIX", 0x19},
    {"SOLARSTRIKER", 7},
    {"X", 0x1C},
    {"GBWARS", 0x15},
};

uint16_t readWord(const std::array<uint8_t, 16 * 7>& command, std::size_t offset)
{
    return static_cast<uint16_t>(command.at(offset)
        | (static_cast<uint16_t>(command.at(offset + 1)) << 8));
}

uint32_t convertColor(uint16_t color)
{
    const uint8_t red = static_cast<uint8_t>(color & 0x1F);
    const uint8_t green = static_cast<uint8_t>((color >> 5) & 0x1F);
    const uint8_t blue = static_cast<uint8_t>((color >> 10) & 0x1F);
    return rgb(static_cast<uint8_t>((red << 3) | (red >> 2)),
        static_cast<uint8_t>((green << 3) | (green >> 2)),
        static_cast<uint8_t>((blue << 3) | (blue >> 2)));
}

}  // namespace

void GbSgb::reset()
{
    command_.fill(0);
    writeIndex_ = 0;
    readyForWrite_ = false;
    readyForStop_ = false;
    disableCommands_ = false;
    lastValue_ = 0;
    ramPalettes_.fill(0);
    attributeMap_.fill(0);
    attributeFiles_.fill(0);
    maskMode_ = 0;
    playerCount_ = 1;
    currentPlayer_ = 0;
    transferCountdown_ = 0;
    transferDestination_ = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        effectivePalettes_.at(i) = BuiltInPalettes.at(i);
    }
    for (std::size_t i = 4; i < effectivePalettes_.size(); ++i) {
        effectivePalettes_.at(i) = BuiltInPalettes.at(i % 4);
    }

    static_assert(std::size(sgb_border_data::palette) <= 16 * 4);
    static_assert(std::size(sgb_border_data::tilemap) == 32 * 28);
    static_assert(sizeof(sgb_border_data::tiles) <= 0x2000);
    borderPalette_.fill(0);
    borderTiles_.fill(0);
    std::copy_n(sgb_border_data::palette,
        std::size(sgb_border_data::palette), borderPalette_.begin());
    std::copy_n(sgb_border_data::tilemap,
        std::size(sgb_border_data::tilemap), borderMap_.begin());
    std::copy_n(sgb_border_data::tiles,
        sizeof(sgb_border_data::tiles), borderTiles_.begin());
}

void GbSgb::setHeader(const GbCartridgeHeader& header)
{
    reset();
    for (const PaletteAssignment& assignment : PaletteAssignments) {
        if (header.title != assignment.title) {
            continue;
        }
        const std::size_t base = static_cast<std::size_t>(assignment.palette) * 4 - 4;
        for (std::size_t i = 0; i < 4; ++i) {
            effectivePalettes_.at(i) = BuiltInPalettes.at(base + i);
        }
        break;
    }
}

void GbSgb::palCommand(std::size_t first, std::size_t second)
{
    const uint16_t color0 = readWord(command_, 1);
    effectivePalettes_.at(0) = color0;
    effectivePalettes_.at(4) = color0;
    effectivePalettes_.at(8) = color0;
    effectivePalettes_.at(12) = color0;
    for (std::size_t i = 0; i < 3; ++i) {
        effectivePalettes_.at(first * 4 + i + 1) = readWord(command_, 3 + i * 2);
        effectivePalettes_.at(second * 4 + i + 1) = readWord(command_, 9 + i * 2);
    }
}

void GbSgb::loadAttributeFile(std::size_t fileIndex)
{
    if (fileIndex > 0x2C) {
        return;
    }
    std::size_t output = 0;
    for (std::size_t i = 0; i < 90; ++i) {
        uint8_t byte = attributeFiles_.at(fileIndex * 90 + i);
        for (int bit = 0; bit < 4; ++bit) {
            attributeMap_.at(output++) = static_cast<uint8_t>(byte >> 6);
            byte = static_cast<uint8_t>(byte << 2);
        }
    }
}

void GbSgb::commandReady()
{
    if ((command_.at(0) & 0xF1) == 0xF1) {
        // The real SGB boot ROM forwards the cartridge header to the SNES.
        // setHeader() already applied the corresponding built-in palette.
        return;
    }
    if ((command_.at(0) & 0x07) == 0) {
        return;
    }

    switch (command_.at(0) >> 3) {
    case CommandPAL01:
        palCommand(0, 1);
        break;
    case CommandPAL23:
        palCommand(2, 3);
        break;
    case CommandPAL03:
        palCommand(0, 3);
        break;
    case CommandPAL12:
        palCommand(1, 2);
        break;
    case CommandATTRBLK: {
        const std::size_t count = std::min<std::size_t>(command_.at(1), 0x12);
        for (std::size_t i = 0; i < count; ++i) {
            const std::size_t base = 2 + i * 6;
            const uint8_t control = command_.at(base);
            const uint8_t palettes = command_.at(base + 1);
            int left = command_.at(base + 2) & 0x1F;
            int top = command_.at(base + 3) & 0x1F;
            int right = command_.at(base + 4) & 0x1F;
            int bottom = command_.at(base + 5) & 0x1F;
            const bool inside = (control & 1) != 0;
            bool middle = (control & 2) != 0;
            const bool outside = (control & 4) != 0;
            const uint8_t insidePalette = palettes & 3;
            uint8_t middlePalette = (palettes >> 2) & 3;
            const uint8_t outsidePalette = (palettes >> 4) & 3;

            if (inside && !middle && !outside) {
                middle = true;
                middlePalette = insidePalette;
            } else if (outside && !middle && !inside) {
                middle = true;
                middlePalette = outsidePalette;
            }

            for (int y = 0; y < 18; ++y) {
                for (int x = 0; x < 20; ++x) {
                    uint8_t& attribute = attributeMap_.at(static_cast<std::size_t>(x + y * 20));
                    if (x < left || x > right || y < top || y > bottom) {
                        if (outside) {
                            attribute = outsidePalette;
                        }
                    } else if (x > left && x < right && y > top && y < bottom) {
                        if (inside) {
                            attribute = insidePalette;
                        }
                    } else if (middle) {
                        attribute = middlePalette;
                    }
                }
            }
        }
        break;
    }
    case CommandATTRCHR: {
        int x = command_.at(1);
        int y = command_.at(2);
        const std::size_t length = readWord(command_, 3);
        const bool vertical = command_.at(5) != 0;
        if (x >= 20 || y >= 18) {
            break;
        }
        const std::size_t maxLength = (16 * 7 - 6) * 4;
        const std::size_t count = std::min(length, maxLength);
        for (std::size_t i = 0; i < count; ++i) {
            const uint8_t palette = static_cast<uint8_t>(
                (command_.at(6 + i / 4) >> (((~i) & 3) << 1)) & 3);
            attributeMap_.at(static_cast<std::size_t>(x + y * 20)) = palette;
            if (vertical) {
                ++y;
                if (y == 18) {
                    ++x;
                    y = 0;
                    if (x == 20) {
                        break;
                    }
                }
            } else {
                ++x;
                if (x == 20) {
                    ++y;
                    x = 0;
                    if (y == 18) {
                        break;
                    }
                }
            }
        }
        break;
    }
    case CommandATTRLIN: {
        const std::size_t count = std::min<std::size_t>(command_.at(1), 16 * 7 - 2);
        for (std::size_t i = 0; i < count; ++i) {
            const uint8_t data = command_.at(2 + i);
            const bool horizontal = (data & 0x80) != 0;
            const uint8_t palette = static_cast<uint8_t>((data >> 5) & 3);
            const int line = data & 0x1F;
            if (horizontal) {
                if (line >= 18) {
                    continue;
                }
                for (int x = 0; x < 20; ++x) {
                    attributeMap_.at(static_cast<std::size_t>(x + line * 20)) = palette;
                }
            } else {
                if (line >= 20) {
                    continue;
                }
                for (int y = 0; y < 18; ++y) {
                    attributeMap_.at(static_cast<std::size_t>(line + y * 20)) = palette;
                }
            }
        }
        break;
    }
    case CommandATTRDIV: {
        const uint8_t lowPalette = command_.at(1) & 3;
        const uint8_t middlePalette = (command_.at(1) >> 2) & 3;
        const uint8_t highPalette = (command_.at(1) >> 4) & 3;
        const bool horizontal = (command_.at(1) & 0x40) != 0;
        const int line = command_.at(2) & 0x1F;
        for (int y = 0; y < 18; ++y) {
            for (int x = 0; x < 20; ++x) {
                const int position = horizontal ? y : x;
                uint8_t& attribute = attributeMap_.at(static_cast<std::size_t>(x + y * 20));
                if (position < line) {
                    attribute = lowPalette;
                } else if (position == line) {
                    attribute = middlePalette;
                } else {
                    attribute = highPalette;
                }
            }
        }
        break;
    }
    case CommandPALSET: {
        const auto loadPalette = [this](std::size_t destination, std::size_t index) {
            const std::size_t source = (index & 0x1FF) * 4;
            for (std::size_t i = 0; i < 4; ++i) {
                effectivePalettes_.at(destination * 4 + i) = ramPalettes_.at(source + i);
            }
        };
        loadPalette(0, command_.at(1) + (command_.at(2) & 1) * 0x100);
        loadPalette(1, command_.at(3) + (command_.at(4) & 1) * 0x100);
        loadPalette(2, command_.at(5) + (command_.at(6) & 1) * 0x100);
        loadPalette(3, command_.at(7) + (command_.at(8) & 1) * 0x100);
        effectivePalettes_.at(4) = effectivePalettes_.at(0);
        effectivePalettes_.at(8) = effectivePalettes_.at(0);
        effectivePalettes_.at(12) = effectivePalettes_.at(0);
        if ((command_.at(9) & 0x80) != 0) {
            loadAttributeFile(command_.at(9) & 0x3F);
        }
        if ((command_.at(9) & 0x40) != 0) {
            maskMode_ = 0;
        }
        break;
    }
    case CommandPALTRN:
        transferDestination_ = TransferPalettes;
        transferCountdown_ = 3;
        break;
    case CommandCHRTRN:
        transferDestination_ = (command_.at(1) & 1) != 0
            ? TransferHighTiles
            : TransferLowTiles;
        transferCountdown_ = 3;
        break;
    case CommandPCTTRN:
        transferDestination_ = TransferBorderData;
        transferCountdown_ = 3;
        break;
    case CommandATTRTRN:
        transferDestination_ = TransferAttributes;
        transferCountdown_ = 3;
        break;
    case CommandDATASND:
        break;
    case CommandMLTREQ:
        playerCount_ = static_cast<uint8_t>((command_.at(1) & 3) + 1);
        if (playerCount_ == 3) {
            ++currentPlayer_;
        }
        currentPlayer_ &= static_cast<uint8_t>(playerCount_ - 1);
        break;
    case CommandATTRSET:
        loadAttributeFile(command_.at(1) & 0x3F);
        if ((command_.at(1) & 0x40) != 0) {
            maskMode_ = 0;
        }
        break;
    case CommandMASKEN:
        maskMode_ = command_.at(1) & 3;
        break;
    default:
        break;
    }
}

void GbSgb::write(uint8_t value)
{
    if (disableCommands_) {
        lastValue_ = value;
        return;
    }

    uint16_t commandSize = static_cast<uint16_t>(
        ((command_.at(0) & 0x07) != 0 ? (command_.at(0) & 0x07) : 1)
        * PacketSize * 8);
    if ((command_.at(0) & 0xF1) == 0xF1) {
        commandSize = PacketSize * 8;
    }

    const uint8_t old = static_cast<uint8_t>((lastValue_ >> 4) & 3);
    if ((value & 0x20) != 0 && (old & 2) == 0) {
        if ((playerCount_ & 1) == 0) {
            ++currentPlayer_;
            currentPlayer_ &= static_cast<uint8_t>(playerCount_ - 1);
        }
    }

    switch ((value >> 4) & 3) {
    case 3:
        if (old == 0) {
            readyForWrite_ = true;
            break;
        }
        if (old == 1 || old == 2) {
            if (!readyForWrite_) {
                break;
            }
            if (readyForStop_) {
                if (writeIndex_ == commandSize) {
                    commandReady();
                    writeIndex_ = 0;
                    command_.fill(0);
                }
                readyForWrite_ = false;
                readyForStop_ = false;
            } else {
                if (writeIndex_ < command_.size() * 8) {
                    if (old == 1) {
                        command_.at(writeIndex_ / 8) |= static_cast<uint8_t>(1U << (writeIndex_ & 7));
                    }
                    ++writeIndex_;
                    if ((writeIndex_ & (PacketSize * 8 - 1)) == 0) {
                        readyForStop_ = true;
                    }
                }
            }
        }
        break;
    case 2:
    case 1:
        // Committed only after a 3 write.
        break;
    case 0:
        readyForWrite_ = false;
        if ((writeIndex_ & (PacketSize * 8 - 1)) != 0 || writeIndex_ == 0
            || readyForStop_) {
            writeIndex_ = 0;
            command_.fill(0);
            readyForStop_ = false;
        }
        break;
    default:
        break;
    }

    lastValue_ = value;
}

void GbSgb::processTransfers(const uint8_t* vram, std::size_t size)
{
    if (transferCountdown_ == 0 || vram == nullptr) {
        return;
    }
    if (--transferCountdown_ != 0) {
        return;
    }

    switch (transferDestination_) {
    case TransferLowTiles:
    case TransferHighTiles:
        if (size >= 0x1000) {
            const std::size_t destination = transferDestination_ == TransferHighTiles
                ? 0x1000
                : 0;
            std::memcpy(borderTiles_.data() + destination, vram, 0x1000);
        }
        break;
    case TransferPalettes:
        if (size >= 0x1000) {
            std::memcpy(ramPalettes_.data(), vram, 0x1000);
        }
        break;
    case TransferBorderData:
        if (size >= 0x860) {
            std::memcpy(borderMap_.data(), vram, 0x700);
            borderPalette_.fill(0);
            std::memcpy(borderPalette_.data(), vram + 0x800, 0x60);
        }
        break;
    case TransferAttributes:
        if (size >= attributeFiles_.size()) {
            std::memcpy(attributeFiles_.data(), vram, attributeFiles_.size());
        }
        break;
    default:
        break;
    }
}

uint8_t GbSgb::attributeForTile(int tileX, int tileY) const
{
    if (tileX < 0 || tileX >= 20 || tileY < 0 || tileY >= 18) {
        return 0;
    }
    return attributeMap_.at(static_cast<std::size_t>(tileX + tileY * 20)) & 3;
}

uint32_t GbSgb::colorForPixel(int x, int y, uint8_t colorIndex) const
{
    if (x < 0 || x >= 160 || y < 0 || y >= 144) {
        return convertColor(effectivePalettes_.at(colorIndex & 3));
    }
    const uint8_t palette = attributeForTile(x / 8, y / 8);
    return convertColor(effectivePalettes_.at(
        static_cast<std::size_t>(palette * 4 + (colorIndex & 3))));
}

void GbSgb::renderBorder(const FrameBuffer& gameFrame, SgbFrameBuffer& output) const
{
    output.pixels.fill(convertColor(effectivePalettes_.at(0)));

    for (int tileY = 0; tileY < 28; ++tileY) {
        for (int tileX = 0; tileX < 32; ++tileX) {
            const uint16_t tile = borderMap_.at(
                static_cast<std::size_t>(tileX + tileY * 32));
            if ((tile & 0x300) != 0) {
                continue;
            }
            const uint8_t palette = static_cast<uint8_t>((tile >> 10) & 3);
            const int flipX = (tile & 0x4000) != 0 ? 0 : 7;
            const int flipY = (tile & 0x8000) != 0 ? 7 : 0;
            for (int y = 0; y < 8; ++y) {
                const std::size_t base = static_cast<std::size_t>(
                    (tile & 0xFF) * 32 + ((y ^ flipY) * 2));
                for (int x = 0; x < 8; ++x) {
                    const uint8_t bit = static_cast<uint8_t>(1U << (x ^ flipX));
                    const int color = ((borderTiles_.at(base) & bit) != 0 ? 1 : 0)
                        | ((borderTiles_.at(base + 1) & bit) != 0 ? 2 : 0)
                        | ((borderTiles_.at(base + 16) & bit) != 0 ? 4 : 0)
                        | ((borderTiles_.at(base + 17) & bit) != 0 ? 8 : 0);
                    const uint32_t value = color == 0
                        ? convertColor(effectivePalettes_.at(0))
                        : convertColor(borderPalette_.at(
                            static_cast<std::size_t>(palette * 16 + color)));
                    output.at(tileX * 8 + x, tileY * 8 + y) = value;
                }
            }
        }
    }

    for (int y = 0; y < FrameBuffer::height; ++y) {
        for (int x = 0; x < FrameBuffer::width; ++x) {
            output.at(x + 48, y + 40) = gameFrame.at(x, y);
        }
    }
}

}  // namespace qnemu
