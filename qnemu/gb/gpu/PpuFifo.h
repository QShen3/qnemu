/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/PpuMemory.h"
#include "qnemu/gb/gpu/PpuRegisters.h"
#include "qnemu/gb/gpu/PpuRenderer.h"

namespace qnemu
{

/**
 * Dot-level background/window pixel FIFO.
 *
 * The fetcher produces eight-pixel rows while the PPU is in mode 3; the FIFO
 * keeps a small pipeline of them so one pixel can be emitted per dot.  Object
 * pixels are composited when the line ends, reusing PpuRenderer's exact sprite
 * priority rules.  This keeps the pixel path and the object path consistent
 * with the reference line renderer while giving the PPU a real per-dot pixel
 * clock.
 */
class PpuFifo
{
public:
    explicit PpuFifo(bool cgb) : renderer_(cgb), cgb_(cgb) {}

    void setCgb(bool cgb)
    {
        cgb_ = cgb;
        renderer_ = PpuRenderer(cgb);
    }

    /// Resets the pipeline at the start of mode 3.
    void startLine(const PpuRegisters& registers,
        const PpuMemory& memory,
        uint8_t line,
        uint16_t windowLine);

    /// Advances one dot.
    void step(const PpuRegisters& registers, const PpuMemory& memory, FrameBuffer& frame);

    bool finished() const { return outputX_ >= FrameBuffer::width; }
    bool windowUsed() const { return windowUsed_; }

    const std::array<uint8_t, FrameBuffer::width>& backgroundColorIndex() const
    {
        return backgroundColorIndex_;
    }
    const std::array<bool, FrameBuffer::width>& backgroundPriority() const
    {
        return backgroundPriority_;
    }

private:
    struct Pixel
    {
        uint8_t colorIndex = 0;
        uint8_t palette = 0;
        bool priority = false;
    };

    struct SpritePenalty
    {
        int triggerX = 0;
        uint8_t dots = 0;
    };

    static constexpr std::size_t fifoCapacity = 16;

    void pushPixel(const Pixel& pixel);
    Pixel popPixel();
    void fillFifo(const PpuRegisters& registers, const PpuMemory& memory);
    void outputPixel(const PpuRegisters& registers, const PpuMemory& memory, FrameBuffer& frame);
    void prepareSpritePenalties(const PpuRegisters& registers, const PpuMemory& memory);
    void applySpritePenalties();

    PpuRenderer renderer_;
    bool cgb_;

    uint8_t line_ = 0;
    uint16_t windowLine_ = 0;

    std::array<Pixel, fifoCapacity> fifo_{};
    std::size_t fifoHead_ = 0;
    std::size_t fifoSize_ = 0;

    std::array<uint8_t, FrameBuffer::width> backgroundColorIndex_{};
    std::array<bool, FrameBuffer::width> backgroundPriority_{};

    std::array<SpritePenalty, 10> spritePenalties_{};

    int outputX_ = 0;
    int windowStartX_ = -1;
    int windowPixelX_ = 0;
    uint16_t startupTicks_ = 0;
    uint8_t stallTicks_ = 0;
    uint8_t fetchCountdown_ = 0;
    bool windowActive_ = false;
    bool windowUsed_ = false;
};

}  // namespace qnemu
