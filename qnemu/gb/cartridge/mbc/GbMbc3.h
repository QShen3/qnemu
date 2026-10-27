/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * MBC3: up to 2 MiB ROM, 32 KiB RAM and the RTC register set.
 *
 * The RTC is exposed through registers 08-0C and reads use the latched
 * snapshot.  This implementation models the register/latch protocol; the
 * clock counter itself currently does not advance with wall time.
 */
class GbMbc3 : public GbMbcInterface
{
public:
    static constexpr uint16_t ramAndTimerEnableAddress = 0x0000;
    static constexpr uint16_t romBankNumberAddress = 0x2000;
    static constexpr uint16_t ramBankOrRtcAddress = 0x4000;
    static constexpr uint16_t latchClockAddress = 0x6000;
    static constexpr uint16_t externalRamStart = 0xA000;
    static constexpr uint16_t externalRamEnd = 0xBFFF;

    GbMbc3() = delete;
    GbMbc3(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc3() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t value) override;
    void step() override;
    void reset() override;

private:
    void latchClock();
    void updateRtcRegisterBytes();
    void advanceSecond();

    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, RamBankSize>> ramBanks;
    uint8_t type;

    bool ramEnabled = false;
    uint8_t romBankNumber = 1;
    uint8_t ramBankOrRtc = 0;
    uint8_t latchSequence = 0;

    uint32_t rtcCycleAccumulator = 0;
    uint8_t rtcSeconds = 0;
    uint8_t rtcMinutes = 0;
    uint8_t rtcHours = 0;
    uint16_t rtcDays = 0;
    bool rtcHalted = false;
    bool rtcCarry = false;
    std::array<uint8_t, 5> rtc{};
    std::array<uint8_t, 5> latchedRtc{};
};

}  // namespace qnemu
