/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/mbc/GbMbc3.h"
#include "qnemu/gb/const.h"

namespace qnemuTest
{

namespace
{

std::vector<std::array<uint8_t, qnemu::RomBankSize>> makeRomBanks(std::size_t count)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> banks(count);
    for (std::size_t bank = 0; bank < count; ++bank) {
        banks.at(bank).fill(static_cast<uint8_t>(bank));
    }
    return banks;
}

}  // namespace

TEST(GbMbc3Test, RomRamAndRtc)
{
    auto romBanks = makeRomBanks(128);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(4);
    qnemu::GbMbc3 mbc(std::move(romBanks), std::move(ramBanks), 0x10);

    mbc.write(qnemu::GbMbc3::romBankNumberAddress, 0x00);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 1);
    mbc.write(qnemu::GbMbc3::romBankNumberAddress, 0x40);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 0x40);

    // RAM is disabled until 0x0A is written.
    EXPECT_EQ(mbc.read(0xA000), 0xFF);
    mbc.write(qnemu::GbMbc3::ramAndTimerEnableAddress, 0x0A);
    mbc.write(qnemu::GbMbc3::ramBankOrRtcAddress, 2);
    mbc.write(0xA123, 0x5A);
    EXPECT_EQ(mbc.read(0xA123), 0x5A);

    // Latch the RTC, then write/read a register through A000.
    mbc.write(qnemu::GbMbc3::ramBankOrRtcAddress, 0x08);
    mbc.write(0xA000, 0x12);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x00);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x01);
    EXPECT_EQ(mbc.read(0xA000), 0x12);

    mbc.write(qnemu::GbMbc3::ramAndTimerEnableAddress, 0x00);
    EXPECT_EQ(mbc.read(0xA000), 0xFF);
}

TEST(GbMbc3Test, SaveRamAndRtc)
{
    auto romBanks = makeRomBanks(4);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(4);
    for (std::size_t bank = 0; bank < ramBanks.size(); ++bank) {
        ramBanks.at(bank).fill(static_cast<uint8_t>(bank + 0x10));
    }
    qnemu::GbMbc3 mbc(std::move(romBanks), std::move(ramBanks), 0x10);

    EXPECT_TRUE(mbc.hasBattery());
    EXPECT_TRUE(mbc.hasRtc());
    const std::vector<uint8_t> dump = mbc.dumpRam();
    EXPECT_EQ(dump.size(), 4 * qnemu::RamBankSize);

    auto otherRomBanks = makeRomBanks(4);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> otherRamBanks(4);
    qnemu::GbMbc3 other(std::move(otherRomBanks), std::move(otherRamBanks), 0x10);
    other.loadRam(dump);
    EXPECT_EQ(other.dumpRam(), dump);

    mbc.loadRtc({0x12, 0x34, 0x05, 0x10, 0x01});
    mbc.advanceRtc(3661);
    const std::vector<uint8_t> rtc = mbc.dumpRtc();
    EXPECT_EQ(rtc.at(0), 0x13);
    EXPECT_EQ(rtc.at(1), 0x35);
    EXPECT_EQ(rtc.at(2), 0x06);
    EXPECT_EQ(rtc.at(3), 0x10);
    EXPECT_EQ(rtc.at(4), 0x01);

    mbc.loadRtc({0, 0, 0, 0, 0x40});  // halted
    mbc.advanceRtc(100);
    EXPECT_EQ(mbc.dumpRtc().at(0), 0);
}

TEST(GbMbc3Test, RtcTicksAndHalts)
{
    auto romBanks = makeRomBanks(4);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);
    qnemu::GbMbc3 mbc(std::move(romBanks), std::move(ramBanks), 0x10);

    mbc.write(qnemu::GbMbc3::ramAndTimerEnableAddress, 0x0A);
    mbc.write(qnemu::GbMbc3::ramBankOrRtcAddress, 0x08);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x00);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x01);
    EXPECT_EQ(mbc.read(0xA000), 0x00);

    for (uint32_t i = 0; i < qnemu::CpuFrequency; ++i) {
        mbc.step();
    }
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x00);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x01);
    EXPECT_EQ(mbc.read(0xA000), 0x01);

    // Halt the RTC and verify that time no longer advances.
    mbc.write(qnemu::GbMbc3::ramBankOrRtcAddress, 0x0C);
    mbc.write(0xA000, 0x40);
    for (uint32_t i = 0; i < qnemu::CpuFrequency * 2; ++i) {
        mbc.step();
    }
    mbc.write(qnemu::GbMbc3::ramBankOrRtcAddress, 0x08);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x00);
    mbc.write(qnemu::GbMbc3::latchClockAddress, 0x01);
    EXPECT_EQ(mbc.read(0xA000), 0x01);
}

}  // namespace qnemuTest
