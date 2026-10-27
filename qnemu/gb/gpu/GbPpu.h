/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/gpu/PpuMemory.h"
#include "qnemu/gb/gpu/PpuRegisters.h"
#include "qnemu/gb/gpu/PpuRenderer.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

/**
 * Game Boy PPU.
 *
 * The PPU owns VRAM/OAM/palette RAM, register state, the dot/line clock and
 * the framebuffer.  It exposes only the memory-mapped interface expected by
 * the MMU, pushes finished frames to DisplayInterface, and reads external
 * memory (ROM/WRAM) through a callback for OAM DMA/HDMA.  It has no Qt or
 * cartridge dependency.
 */
class GbPpu : public GbGpuInterface
{
public:
    GbPpu(bool cgb,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~GbPpu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t currentMode() const override;
    bool isLcdEnable() const override;

    /// External memory used by OAM DMA/HDMA (ROM, cartridge RAM, WRAM).
    void setExternalRead(std::function<uint8_t(uint16_t)> reader);
    void setCgb(bool cgb);

    const FrameBuffer& frameBuffer() const { return frame_; }
    uint64_t frameCount() const { return frameCount_; }

private:
    void stepOamDma();
    void stepHdma();
    void stepTiming();
    void setMode(uint8_t mode);
    void updateLyc();
    void updateStatIrq();
    void requestLcdInterrupt();
    void renderScanline();
    uint16_t mode3Length() const;

    void startOamDma(uint8_t source);
    uint8_t readDmaSource(uint16_t address) const;
    void transferHdmaBlock();
    void writeHdma5(uint8_t value);

    void writeLcdc(uint8_t value);
    void writeStat(uint8_t value);
    void writeLy(uint8_t value);

    bool cgb_;
    std::shared_ptr<DisplayInterface> display_;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler_;
    std::function<uint8_t(uint16_t)> externalRead_;

    PpuRegisters registers_;
    PpuMemory memory_;
    PpuRenderer renderer_;

    FrameBuffer frame_;
    uint16_t dot_ = 0;
    uint16_t mode0StartDot_ = 252;
    uint16_t windowLine_ = 0;
    uint64_t frameCount_ = 0;
    bool statIrqLine_ = false;
    bool lcdJustEnabled_ = false;

    bool oamDmaActive_ = false;
    uint8_t oamDmaRegister_ = 0x00;
    uint16_t oamDmaSource_ = 0x0000;
    uint16_t oamDmaPendingSource_ = 0x0000;
    uint16_t oamDmaIndex_ = 0;
    uint8_t oamDmaTicks_ = 0;
    uint8_t oamDmaStartupTicks_ = 0;

    bool hdmaActive_ = false;
    bool hdmaHblank_ = false;
    uint8_t hdma5_ = 0xFF;
    uint8_t hdmaRemaining_ = 0;
    uint8_t hdmaTicks_ = 0;
    uint16_t hdmaSource_ = 0x0000;
    uint16_t hdmaDestination_ = 0x8000;
};

}  // namespace qnemu
