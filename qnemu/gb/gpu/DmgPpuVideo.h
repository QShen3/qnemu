/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/gb/gpu/PpuFifo.h"
#include "qnemu/gb/gpu/PpuRenderer.h"
#include "qnemu/gb/gpu/PpuVideo.h"

namespace qnemu
{

/// DMG video path: one VRAM bank, BGP/OBP palettes, no CGB registers.
class DmgPpuVideo final : public PpuVideo
{
public:
    DmgPpuVideo();

    void reset() override;

    bool readRegister(uint16_t address, const PpuRegisters& /*registers*/,
        const PpuMemory& /*memory*/, uint8_t& value) const override
    {
        // The DMG has no CGB video registers; they read back as $FF.
        if (address == 0xFF4F || (address >= 0xFF51 && address <= 0xFF55)
            || (address >= 0xFF68 && address <= 0xFF6C)) {
            value = 0xFF;
            return true;
        }
        return false;
    }
    bool writeRegister(uint16_t address, uint8_t /*value*/,
        PpuRegisters& /*registers*/, PpuMemory& /*memory*/) override
    {
        return address == 0xFF4F || (address >= 0xFF51 && address <= 0xFF55)
            || (address >= 0xFF68 && address <= 0xFF6C);
    }

    void setExternalRead(std::function<uint8_t(uint16_t)> /*reader*/) override {}
    void setColorizer(PpuColorizer* colorizer) override;

    void stepHdma(PpuMemory& /*memory*/) override {}
    void onMode0(PpuMemory& /*memory*/) override {}
    bool isHdmaActive() const override { return false; }

    void startLine(PpuRegisters& registers, PpuMemory& memory,
        uint8_t line, uint16_t windowLine) override;
    void step(PpuRegisters& registers, PpuMemory& memory, FrameBuffer& frame) override;
    bool finished() const override { return fifo_.finished(); }
    bool windowUsed() const override { return fifo_.windowUsed(); }
    const std::array<uint8_t, FrameBuffer::width>& backgroundColorIndex() const override
    {
        return fifo_.backgroundColorIndex();
    }
    const std::array<bool, FrameBuffer::width>& backgroundPriority() const override
    {
        return fifo_.backgroundPriority();
    }

    void renderSprites(PpuRegisters& registers, PpuMemory& memory,
        uint8_t line, FrameBuffer& frame) override;

private:
    PpuFifo fifo_;
    PpuRenderer renderer_;
};

}  // namespace qnemu
