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
#include "qnemu/gb/gpu/PpuColorizer.h"
#include "qnemu/gb/gpu/PpuFrameSink.h"
#include "qnemu/gb/gpu/PpuMemory.h"
#include "qnemu/gb/gpu/PpuRegisters.h"
#include "qnemu/gb/gpu/PpuVideo.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

/**
 * Game Boy PPU timing core.
 *
 * The PPU owns VRAM/OAM, register state, the dot/line clock and the
 * framebuffer.  Platform-specific video behavior (DMG palettes, CGB palette
 * RAM/HDMA or SGB colorization) lives in a PpuVideo object.
 */
class GbPpu : public GbGpuInterface
{
public:
    GbPpu(std::unique_ptr<PpuVideo> video,
        std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~GbPpu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    /// Power-on state used before executing a boot ROM: the LCD is off, so
    /// the boot ROM can clear VRAM/OAM and load its logo unobstructed.
    void resetForBootRom();

    uint8_t currentMode() const override;
    bool isLcdEnable() const override;
    bool isHdmaActive() const override { return video_->isHdmaActive(); }

    /// External memory used by OAM DMA/HDMA (ROM, cartridge RAM, WRAM).
    void setExternalRead(std::function<uint8_t(uint16_t)> reader);
    void setColorizer(PpuColorizer* colorizer);
    void setFrameSink(PpuFrameSink* frameSink);

    /// CGB compatibility rendering mode.  The DMG PPU ignores it.
    virtual void setDmgMode(bool /*dmgMode*/) {}
    virtual void enableDmgCompatibility() {}

    /// DMG OAM corruption quirk.  CGB hardware and SGB models that use the
    /// CGB PPU path must leave this disabled.
    void setOamBugEnabled(bool enabled) { oamBugEnabled_ = enabled; }
    int accessedOamRow() const { return accessedOamRow_; }
    void triggerOamBugWrite(uint16_t address, int rowOffset = 0) override;
    void triggerOamBugRead(uint16_t address) override;
    void triggerOamBugReadIncrease(uint16_t address) override;

    const FrameBuffer& frameBuffer() const { return frame_; }
    uint64_t frameCount() const { return frameCount_; }

protected:
    PpuVideo& video() const { return *video_; }
    PpuMemory& memory() { return memory_; }

private:
    void stepOamDma();
    void stepTiming();
    void setMode(uint8_t mode);
    void updateLyc();
    void updateStatIrq();
    void requestLcdInterrupt();
    bool oamAccessBlocked() const;
    bool oamReadBlocked() const;
    bool vramAccessBlocked() const;
    bool vramReadBlocked() const;
    bool firstLineAccessBlocked() const;
    void finishMode3();

    void startOamDma(uint8_t source);
    uint8_t readDmaSource(uint16_t address) const;

    void writeLcdc(uint8_t value);
    void writeStat(uint8_t value);
    void writeLy(uint8_t value);

    std::shared_ptr<DisplayInterface> display_;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler_;
    std::function<uint8_t(uint16_t)> externalRead_;

    PpuRegisters registers_;
    PpuMemory memory_;
    std::unique_ptr<PpuVideo> video_;

    FrameBuffer frame_;
    uint16_t dot_ = 0;
    uint16_t lineLength_ = 456;
    uint8_t mode3Dots_ = 0;
    uint16_t windowLine_ = 0;
    uint64_t frameCount_ = 0;
    bool oamBugEnabled_ = false;
    PpuFrameSink* frameSink_ = nullptr;
    int accessedOamRow_ = -1;
    bool statIrqLine_ = false;
    bool lcdJustEnabled_ = false;

    bool oamDmaActive_ = false;
    uint8_t oamDmaRegister_ = 0x00;
    uint16_t oamDmaSource_ = 0x0000;
    uint16_t oamDmaPendingSource_ = 0x0000;
    uint16_t oamDmaIndex_ = 0;
    uint8_t oamDmaTicks_ = 0;
    uint8_t oamDmaStartupTicks_ = 0;
};

}  // namespace qnemu
