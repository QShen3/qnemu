/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cpu/CpuBus.h"
#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "tests/support/MemoryDisplay.h"

namespace qnemuTest
{

enum class BootModel
{
    dmg0,
    dmgAbc,
    mgb,
    sgb,
    sgb2,
};

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

    /// Load a ROM image from memory, with optional test-specific patches.
    bool loadRomData(const std::vector<uint8_t>& romData, bool cgb,
        bool enablePpu = false, bool enableApu = false);

    /// Execute the bundled QNEMU CGB boot ROM instead of applying post-boot
    /// state.  Call before loadRom()/loadRomData().
    void enableBootRom();

    /// Reproduce the model-specific post-boot I/O state without executing a
    /// boot ROM.  CPU registers are applied by the runner.
    void applyBootState(BootModel model);

    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    void tick(uint8_t cycles) override;
    void triggerOamBugWrite(uint16_t address, int rowOffset = 0) override;
    void triggerOamBugRead(uint16_t address) override;
    void triggerOamBugReadIncrease(uint16_t address) override;
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
    bool finishLoad(bool cgb, bool enablePpu, bool enableApu);
    void reset();
    void ppuTick();

    std::array<uint8_t, 0x10000> memory_{};

    bool cgb_ = true;
    bool cgbHardware_ = true;
    std::span<const uint8_t> bootRom_;
    bool bootRomEnabled_ = false;
    bool bootRomActive_ = false;
    const qnemu::GbMbcFactory mbcFactory;
    std::unique_ptr<qnemu::GbCartridge> cartridge;

    std::shared_ptr<qnemu::GbInterruptHandler> interruptHandler =
        std::make_shared<qnemu::GbInterruptHandler>();
    std::unique_ptr<qnemu::GbTimer> timer;

    uint8_t lcdc_ = 0x91;
    uint8_t stat_ = 0x80;
    uint8_t scy_ = 0;
    uint8_t scx_ = 0;
    uint8_t ly_ = 0;
    uint8_t lyc_ = 0;
    uint8_t bgp_ = 0xFC;
    uint8_t obp0_ = 0xFF;
    uint8_t obp1_ = 0xFF;
    uint8_t wy_ = 0;
    uint8_t wx_ = 0;
    uint8_t joypad_ = 0xCF;
    uint16_t ppuDivider_ = 0;
    bool freezeLy_ = false;
    bool serialTimingEnabled_ = false;
    uint16_t serialCyclesRemaining_ = 0;
    uint16_t serialClockPhase_ = 50;

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
