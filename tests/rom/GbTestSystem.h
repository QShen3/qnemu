/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "qnemu/gb/cpu/CpuBus.h"

namespace qnemuTest
{

/**
 * Minimal headless Game Boy system used to run CPU test ROMs.
 *
 * It intentionally implements only what the CPU/timer/interrupt/serial tests
 * need: ROM (with basic MBC1 banking), RAM, DIV/TIMA/TMA/TAC, IF/IE, a line
 * counter for VBlank interrupts, and the link-port serial output convention.
 * PPU mode timing, OAM DMA, APU and the real MMU are out of scope here; those
 * are exercised by the main emulator instead.
 */
class GbTestSystem : public qnemu::CpuBus
{
public:
    bool loadRom(const std::string& path, bool cgb);

    uint8_t read(uint16_t address) override;
    void write(uint16_t address, uint8_t value) override;
    void tick(uint8_t cycles) override;
    bool onStop() override;

    const std::string& serialOutput() const { return serialOutput_; }

    /// Decode the BG tilemap for the simple ASCII console used by some
    /// blargg ROMs.  This is only a diagnostic aid; it is not a renderer.
    std::string screenText() const;

    uint64_t cycles() const { return cycles_; }

private:
    void reset();
    void timerTick();
    void ppuTick();
    uint8_t timerClockBit() const;
    void updateTimerClockBit();
    uint32_t romOffset(uint16_t address) const;

    std::vector<uint8_t> rom_;
    std::array<uint8_t, 0x10000> memory_{};

    bool cgb_ = true;
    bool mbc1_ = false;
    bool ramEnabled_ = false;
    uint8_t romBankLow_ = 1;
    uint8_t ramBank_ = 0;
    uint8_t bankingMode_ = 0;

    uint16_t divider_ = 0xAB00;
    uint8_t tima_ = 0;
    uint8_t tma_ = 0;
    uint8_t tac_ = 0xF8;
    uint8_t previousTimerBit_ = 0;
    bool timaOverflow_ = false;
    uint8_t overflowCycles_ = 0;

    uint8_t interruptFlag_ = 0xE1;
    uint8_t interruptEnabled_ = 0x00;

    uint8_t lcdc_ = 0x91;
    uint8_t stat_ = 0x80;
    uint8_t ly_ = 0;
    uint8_t lyc_ = 0;
    uint16_t ppuDivider_ = 0;

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
