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

class GbSgb;

/**
 * Stateless pixel/scanline renderer.  It only reads PPU memory/registers and
 * writes into the framebuffer, so it can be unit-tested with synthetic VRAM
 * and OAM data without running a CPU or a clock.  PpuFifo uses the per-pixel
 * background fetch and the object compositor; the full-line entry point is
 * kept as the reference renderer used by the unit tests.
 */
class PpuRenderer
{
public:
    explicit PpuRenderer(bool cgb, GbSgb* sgb = nullptr, bool dmgMode = false)
        : cgb_(cgb), sgb_(sgb), dmgMode_(dmgMode) {}

    void setSgb(GbSgb* sgb) { sgb_ = sgb; }
    void setDmgMode(bool dmgMode) { dmgMode_ = dmgMode; }

    /// Returns true when the window contributed at least one pixel to the line.
    bool renderLine(const PpuRegisters& registers,
        const PpuMemory& memory,
        uint8_t line,
        uint16_t windowLine,
        FrameBuffer& frame) const;

    struct BackgroundPixel
    {
        uint8_t colorIndex = 0;
        uint8_t palette = 0;
        bool priority = false;
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

    /// Converts a DMG color index through BGP/OBP and, in SGB mode, through
    /// the SGB attribute palette for the screen pixel.
    uint32_t dmgPixelColor(const PpuMemory& memory,
        int x,
        int y,
        uint8_t colorIndex,
        uint8_t palette) const;

private:
    struct Sprite
    {
        uint8_t oamIndex = 0;
        int x = 0;
        int y = 0;
    };

    static uint16_t tileDataAddress(const PpuRegisters& registers, uint8_t tileIndex);

    bool cgbRendering() const { return cgb_ && !dmgMode_; }

    bool cgb_;
    GbSgb* sgb_ = nullptr;
    bool dmgMode_ = false;
};

}  // namespace qnemu
