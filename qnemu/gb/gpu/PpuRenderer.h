/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/PpuMemory.h"
#include "qnemu/gb/gpu/PpuRegisters.h"

namespace qnemu
{

/**
 * Stateless scanline renderer.  It only reads PPU memory/registers and writes
 * one row of the framebuffer, so it can be unit-tested with synthetic VRAM and
 * OAM data without running a CPU or a clock.
 */
class PpuRenderer
{
public:
    explicit PpuRenderer(bool cgb) : cgb_(cgb) {}

    /// Returns true when the window contributed at least one pixel to the line.
    bool renderLine(const PpuRegisters& registers,
        const PpuMemory& memory,
        uint8_t line,
        uint16_t windowLine,
        FrameBuffer& frame) const;

private:
    struct BackgroundPixel
    {
        uint8_t colorIndex = 0;
        uint8_t palette = 0;
        bool priority = false;
    };

    struct Sprite
    {
        uint8_t oamIndex = 0;
        int x = 0;
        int y = 0;
    };

    BackgroundPixel fetchBackgroundPixel(const PpuRegisters& registers,
        const PpuMemory& memory,
        uint8_t x,
        uint8_t y,
        bool window) const;

    void renderSprites(const PpuRegisters& registers,
        const PpuMemory& memory,
        uint8_t line,
        const std::array<uint8_t, FrameBuffer::width>& backgroundColorIndex,
        const std::array<bool, FrameBuffer::width>& backgroundPriority,
        FrameBuffer& frame) const;

    static uint16_t tileDataAddress(const PpuRegisters& registers, uint8_t tileIndex);

    bool cgb_;
};

}  // namespace qnemu
