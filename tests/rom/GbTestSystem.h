/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/cpu/CpuBus.h"
#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "tests/support/MemoryDisplay.h"

namespace qnemuTest
{

/**
 * Minimal headless Game Boy system used to run test ROMs.
 *
 * The CPU/timer/serial path is always available.  A real GbPpu can optionally
 * be attached for rendering tests such as dmg-acid2/cgb-acid2; when it is not
 * attached the system keeps a tiny LY/VBlank stub so CPU-only tests stay fast
 * and independent of PPU timing.
 */
class GbTestSystem : public qnemu::CpuBus
{
public:
    bool loadRom(const std::string& path, bool cgb, bool enablePpu = false,
        bool enableApu = false);

    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    void tick(uint8_t cycles) override;
    bool onStop() override;

    const std::string& serialOutput() const { return serialOutput_; }

    /// Decode the BG tilemap for the simple ASCII console used by some
    /// blargg ROMs.  This is only a diagnostic aid; it is not a renderer.
    std::string screenText();

    const qnemu::FrameBuffer* frameBuffer() const
    {
        return ppu ? &ppu->frameBuffer() : nullptr;
    }

    uint64_t frameCount() const
    {
        return ppu ? ppu->frameCount() : 0;
    }

    uint64_t cycles() const { return cycles_; }

private:
    void reset();
    void ppuTick();
    uint32_t romOffset(uint16_t address) const;

    std::vector<uint8_t> rom_;
    std::array<uint8_t, 0x10000> memory_{};

    bool cgb_ = true;
    bool mbc1_ = false;
    bool ramEnabled_ = false;
    uint8_t romBankLow_ = 1;
    uint8_t ramBank_ = 0;
    uint8_t bankingMode_ = 0;

    std::shared_ptr<qnemu::GbInterruptHandler> interruptHandler =
        std::make_shared<qnemu::GbInterruptHandler>();
    std::unique_ptr<qnemu::GbTimer> timer;

    uint8_t lcdc_ = 0x91;
    uint8_t stat_ = 0x80;
    uint8_t ly_ = 0;
    uint8_t lyc_ = 0;
    uint16_t ppuDivider_ = 0;

    std::shared_ptr<MemoryDisplay> display;
    std::unique_ptr<qnemu::GbPpu> ppu;
    std::unique_ptr<qnemu::GbApu> apu;

    uint8_t serialData_ = 0;
    uint8_t serialControl_ = 0x00;
    std::string serialOutput_;

    uint8_t nr52_ = 0x80;

    bool doubleSpeed_ = false;
    bool speedSwitchArmed_ = false;
    uint8_t speedRemainder_ = 0;

    uint64_t cycles_ = 0;
};

}  // namespace qnemuTest
