/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/PpuColorizer.h"

namespace qnemu
{

class PpuMemory;
struct PpuRegisters;

/**
 * Platform-specific PPU video path.
 *
 * The PPU owns the dot clock, mode transitions, interrupts, OAM DMA and the
 * framebuffer.  A video object owns the pixel FIFO/renderer, the CGB-only
 * registers and HDMA, so the DMG PPU never contains CGB behavior.
 */
class PpuVideo
{
public:
    virtual ~PpuVideo() = default;

    virtual void reset() = 0;

    /// Handles a CGB-only register.  Returns false for addresses the video
    /// object does not own.
    virtual bool readRegister(uint16_t address, const PpuRegisters& registers,
        const PpuMemory& memory, uint8_t& value) const = 0;
    virtual bool writeRegister(uint16_t address, uint8_t value,
        PpuRegisters& registers, PpuMemory& memory) = 0;

    virtual void setExternalRead(std::function<uint8_t(uint16_t)> reader) = 0;
    virtual void setColorizer(PpuColorizer* colorizer) = 0;

    virtual void stepHdma(PpuMemory& memory) = 0;
    virtual void onMode0(PpuMemory& memory) = 0;
    virtual bool isHdmaActive() const = 0;

    virtual void startLine(PpuRegisters& registers, PpuMemory& memory,
        uint8_t line, uint16_t windowLine) = 0;
    virtual void step(PpuRegisters& registers, PpuMemory& memory,
        FrameBuffer& frame) = 0;
    virtual bool finished() const = 0;
    virtual bool windowUsed() const = 0;
    virtual const std::array<uint8_t, FrameBuffer::width>& backgroundColorIndex() const = 0;
    virtual const std::array<bool, FrameBuffer::width>& backgroundPriority() const = 0;

    virtual void renderSprites(PpuRegisters& registers, PpuMemory& memory,
        uint8_t line, FrameBuffer& frame) = 0;
};

}  // namespace qnemu
