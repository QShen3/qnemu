/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/mbc/GbMbc5.h"
#include "qnemu/gb/const.h"

namespace qnemuTest
{

namespace
{

std::vector<std::array<uint8_t, qnemu::RomBankSize>> makeRomBanks(std::size_t count)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> banks(count);
    for (std::size_t bank = 0; bank < count; ++bank) {
        banks.at(bank).fill(static_cast<uint8_t>(bank & 0xFF));
    }
    return banks;
}

}  // namespace

TEST(GbMbc5Test, RomAndRamBanking)
{
    auto romBanks = makeRomBanks(512);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(16);
    qnemu::GbMbc5 mbc(std::move(romBanks), std::move(ramBanks), 0x1B);

    // MBC5 allows bank 0 at 4000-7FFF.
    mbc.write(qnemu::GbMbc5::romBankLowAddress, 0x00);
    mbc.write(qnemu::GbMbc5::romBankHighAddress, 0x00);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 0x00);

    mbc.write(qnemu::GbMbc5::romBankLowAddress, 0x34);
    mbc.write(qnemu::GbMbc5::romBankHighAddress, 0x01);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 0x34);

    // RAM enable + bank selection.
    EXPECT_EQ(mbc.read(0xA000), 0xFF);
    mbc.write(qnemu::GbMbc5::ramEnableAddress, 0x0A);
    mbc.write(qnemu::GbMbc5::ramBankNumberAddress, 0x03);
    mbc.write(0xA000, 0x5A);
    EXPECT_EQ(mbc.read(0xA000), 0x5A);

    mbc.write(qnemu::GbMbc5::ramBankNumberAddress, 0x04);
    EXPECT_EQ(mbc.read(0xA000), 0x00);
}

TEST(GbMbc5Test, SaveRamAndBatteryFlag)
{
    auto romBanks = makeRomBanks(4);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(4);
    for (std::size_t bank = 0; bank < ramBanks.size(); ++bank) {
        ramBanks.at(bank).fill(static_cast<uint8_t>(bank + 1));
    }
    qnemu::GbMbc5 mbc(std::move(romBanks), std::move(ramBanks), 0x1B);

    EXPECT_TRUE(mbc.hasBattery());
    const std::vector<uint8_t> dump = mbc.dumpRam();
    EXPECT_EQ(dump.size(), 4 * qnemu::RamBankSize);

    auto otherRomBanks = makeRomBanks(4);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> otherRamBanks(4);
    qnemu::GbMbc5 other(std::move(otherRomBanks), std::move(otherRamBanks), 0x1B);
    other.loadRam(dump);
    EXPECT_EQ(other.dumpRam(), dump);

    auto noBatteryRomBanks = makeRomBanks(4);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> noBatteryRamBanks(1);
    qnemu::GbMbc5 noBattery(std::move(noBatteryRomBanks),
        std::move(noBatteryRamBanks), 0x1A);
    EXPECT_FALSE(noBattery.hasBattery());
}

TEST(GbMbc5Test, RumbleBitIsNotPartOfRamBank)
{
    auto romBanks = makeRomBanks(64);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(8);
    qnemu::GbMbc5 mbc(std::move(romBanks), std::move(ramBanks), 0x1D);

    mbc.write(qnemu::GbMbc5::ramEnableAddress, 0x0A);
    mbc.write(qnemu::GbMbc5::ramBankNumberAddress, 0x01);
    mbc.write(0xA000, 0x11);
    mbc.write(qnemu::GbMbc5::ramBankNumberAddress, 0x09);  // rumble + bank 1
    EXPECT_EQ(mbc.read(0xA000), 0x11);
}

}  // namespace qnemuTest
