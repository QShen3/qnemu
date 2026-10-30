/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/PpuFifo.h"

namespace qnemu
{

void PpuFifo::startLine(const PpuRegisters& registers,
    const PpuMemory& memory,
    uint8_t line,
    uint16_t windowLine)
{
    line_ = line;
    windowLine_ = windowLine;

    fifoHead_ = 0;
    fifoSize_ = 0;
    backgroundColorIndex_.fill(0);
    backgroundPriority_.fill(false);

    outputX_ = 0;
    windowPixelX_ = 0;
    windowActive_ = false;
    windowUsed_ = false;
    stallTicks_ = 0;
    fetchCountdown_ = 0;

    const uint8_t fineScroll = static_cast<uint8_t>(registers.scx & 0x07);
    uint16_t scrollPenalty = 0;
    if (fineScroll != 0) {
        // The fetcher advances in pairs of dots, so a partial tile is rounded
        // up to 4 or 8 dots depending on how much of it is discarded.
        scrollPenalty = fineScroll <= 4 ? 4 : 8;
    }
    startupTicks_ = static_cast<uint16_t>(14 + scrollPenalty);

    const bool backgroundVisible = cgb_ || registers.bgWindowEnabled();
    windowStartX_ = -1;
    if (backgroundVisible && registers.windowEnabled()
        && registers.wx >= 7 && registers.wx <= 166
        && line >= registers.wy) {
        windowStartX_ = static_cast<int>(registers.wx) - 7;
    }

    prepareSpritePenalties(registers, memory);
}

void PpuFifo::pushPixel(const Pixel& pixel)
{
    if (fifoSize_ >= fifoCapacity) {
        return;
    }
    const std::size_t index = (fifoHead_ + fifoSize_) % fifoCapacity;
    fifo_.at(index) = pixel;
    ++fifoSize_;
}

PpuFifo::Pixel PpuFifo::popPixel()
{
    Pixel pixel = fifo_.at(fifoHead_);
    fifoHead_ = (fifoHead_ + 1) % fifoCapacity;
    --fifoSize_;
    return pixel;
}

void PpuFifo::fillFifo(const PpuRegisters& registers, const PpuMemory& memory)
{
    if (fifoSize_ + 8 > fifoCapacity) {
        return;
    }

    const bool backgroundVisible = cgb_ || registers.bgWindowEnabled();
    const int backgroundBase = outputX_ + static_cast<int>(fifoSize_);
    for (int index = 0; index < 8; ++index) {
        Pixel pixel;
        if (backgroundVisible) {
            PpuRenderer::BackgroundPixel fetched;
            if (windowActive_) {
                const uint8_t windowX = static_cast<uint8_t>(windowPixelX_ & 0xFF);
                const uint8_t windowY = static_cast<uint8_t>(windowLine_ & 0xFF);
                fetched = renderer_.fetchBackgroundPixel(registers, memory, windowX, windowY, true);
                ++windowPixelX_;
            } else {
                const int screenX = backgroundBase + index;
                const uint8_t backgroundX = static_cast<uint8_t>((screenX + registers.scx) & 0xFF);
                const uint8_t backgroundY = static_cast<uint8_t>((line_ + registers.scy) & 0xFF);
                fetched = renderer_.fetchBackgroundPixel(registers, memory, backgroundX, backgroundY, false);
            }
            pixel.colorIndex = fetched.colorIndex;
            pixel.palette = fetched.palette;
            pixel.priority = fetched.priority;
        }
        pushPixel(pixel);
    }
}

void PpuFifo::outputPixel(const PpuRegisters& registers,
    const PpuMemory& memory,
    FrameBuffer& frame)
{
    const Pixel pixel = popPixel();
    const bool backgroundVisible = cgb_ || registers.bgWindowEnabled();
    const uint8_t colorIndex = backgroundVisible ? pixel.colorIndex : 0;

    backgroundColorIndex_.at(static_cast<std::size_t>(outputX_)) = colorIndex;
    backgroundPriority_.at(static_cast<std::size_t>(outputX_)) = backgroundVisible && pixel.priority;

    if (cgb_) {
        frame.at(outputX_, line_) = memory.cgbColor(false, pixel.palette, colorIndex);
    } else if (memory.dmgCompatibility()) {
        frame.at(outputX_, line_) =
            memory.dmgCompatibilityColor(false, 0, registers.bgp, colorIndex);
    } else {
        frame.at(outputX_, line_) = memory.dmgColor(registers.bgp, colorIndex);
    }
    ++outputX_;
}

void PpuFifo::prepareSpritePenalties(const PpuRegisters& registers, const PpuMemory& memory)
{
    spritePenalties_.fill(SpritePenalty{});

    const int spriteHeight = registers.objSize16() ? 16 : 8;
    std::array<bool, 21> tilesSeen{};
    bool leftEdgePenaltyApplied = false;
    std::size_t penaltyCount = 0;
    int spriteCount = 0;

    for (uint8_t index = 0; index < OamSize && spriteCount < 10; index += 4) {
        const int spriteY = static_cast<int>(memory.readOamByte(index)) - 16;
        if (static_cast<int>(line_) < spriteY
            || static_cast<int>(line_) >= spriteY + spriteHeight) {
            continue;
        }

        const uint8_t oamX = memory.readOamByte(static_cast<uint8_t>(index + 1));
        const int screenX = static_cast<int>(oamX) - 8;
        if (screenX >= FrameBuffer::width) {
            continue;
        }

        ++spriteCount;
        int penalty = 6;
        if (oamX < 8) {
            // Objects completely or partially left of the screen share the
            // first tile fetch.  The wait is longest for X=0 (5 dots) and
            // shrinks by one dot per pixel to the right, once per line.
            if (!leftEdgePenaltyApplied) {
                penalty += std::max(0, 5 - (oamX & 0x07));
                leftEdgePenaltyApplied = true;
            }
        } else {
            const int scrolledX = screenX + registers.scx;
            const int tile = std::min(20, scrolledX >> 3);
            if (!tilesSeen.at(static_cast<std::size_t>(tile))) {
                const int pixel = scrolledX & 0x07;
                penalty += std::max(0, 5 - pixel);
                tilesSeen.at(static_cast<std::size_t>(tile)) = true;
            }
        }

        if (penaltyCount < spritePenalties_.size()) {
            spritePenalties_.at(penaltyCount) = SpritePenalty {
                std::max(0, screenX),
                static_cast<uint8_t>(penalty),
            };
            ++penaltyCount;
        }
    }
}

void PpuFifo::applySpritePenalties()
{
    uint8_t dots = 0;
    for (SpritePenalty& penalty : spritePenalties_) {
        if (penalty.dots != 0 && penalty.triggerX <= outputX_) {
            dots = static_cast<uint8_t>(dots + penalty.dots);
            penalty.dots = 0;
        }
    }
    if (dots != 0) {
        stallTicks_ = dots;
    }
}

void PpuFifo::step(const PpuRegisters& registers, const PpuMemory& memory, FrameBuffer& frame)
{
    if (finished()) {
        return;
    }

    if (startupTicks_ > 0) {
        --startupTicks_;
        if (startupTicks_ > 0) {
            return;
        }
        fillFifo(registers, memory);
        if (fifoSize_ == 0) {
            return;
        }
        fetchCountdown_ = 8;
    }

    if (stallTicks_ > 0) {
        --stallTicks_;
        return;
    }

    if (!windowActive_ && windowStartX_ >= 0 && outputX_ >= windowStartX_) {
        windowActive_ = true;
        windowUsed_ = true;
        fifoHead_ = 0;
        fifoSize_ = 0;
        windowPixelX_ = 0;
        fetchCountdown_ = 0;
        stallTicks_ = 6;
        --stallTicks_;
        return;
    }

    applySpritePenalties();
    if (stallTicks_ > 0) {
        --stallTicks_;
        return;
    }

    if (fifoSize_ == 0) {
        fillFifo(registers, memory);
        if (fifoSize_ == 0) {
            return;
        }
        if (fetchCountdown_ == 0) {
            fetchCountdown_ = 8;
        }
    }

    outputPixel(registers, memory, frame);

    if (fetchCountdown_ > 0) {
        --fetchCountdown_;
        if (fetchCountdown_ == 0) {
            if (fifoSize_ <= 8) {
                fillFifo(registers, memory);
            }
            fetchCountdown_ = 8;
        }
    }
}

}  // namespace qnemu
