/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <functional>

#include "qnemu/gb/gpu/PpuFifo.h"
#include "qnemu/gb/gpu/PpuRenderer.h"
#include "qnemu/gb/gpu/PpuVideo.h"

namespace qnemu
{

/// CGB video path: VRAM banks, palette RAM, HDMA and the DMG compatibility
/// rendering mode selected by KEY0.
class CgbPpuVideo final : public PpuVideo
{
public:
    CgbPpuVideo();

    void reset() override;

    bool readRegister(uint16_t address, const PpuRegisters& registers,
        const PpuMemory& memory, uint8_t& value) const override;
    bool writeRegister(uint16_t address, uint8_t value,
        PpuRegisters& registers, PpuMemory& memory) override;

    void setExternalRead(std::function<uint8_t(uint16_t)> reader) override;
    void setColorizer(PpuColorizer* colorizer) override;

    void stepHdma(PpuMemory& memory) override;
    void onMode0(PpuMemory& memory) override;
    bool isHdmaActive() const override { return hdmaActive_ && !hdmaHblank_; }

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

    void setDmgMode(bool dmgMode);

private:
    void transferHdmaBlock(PpuMemory& memory);
    void writeHdma5(uint8_t value);

    PpuFifo fifo_;
    PpuRenderer renderer_;
    std::function<uint8_t(uint16_t)> externalRead_;
    bool dmgMode_ = false;

    bool hdmaActive_ = false;
    bool hdmaHblank_ = false;
    uint8_t hdma5_ = 0xFF;
    uint8_t hdmaRemaining_ = 0;
    uint8_t hdmaTicks_ = 0;
    uint16_t hdmaSource_ = 0x0000;
    uint16_t hdmaDestination_ = 0x8000;
};

}  // namespace qnemu
