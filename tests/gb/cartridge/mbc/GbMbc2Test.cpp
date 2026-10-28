/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/mbc/GbMbc2.h"
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

TEST(GbMbc2Test, RomAndRamBanking)
{
    auto romBanks = makeRomBanks(16);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks;
    qnemu::GbMbc2 mbc(std::move(romBanks), std::move(ramBanks), 0x05);

    // Bank register uses A8=1; bank 0 maps to 1.
    mbc.write(0x2100, 0x00);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 1);
    mbc.write(0x2100, 0x07);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 7);

    // RAM is disabled by default.
    EXPECT_EQ(mbc.read(0xA000), 0xFF);

    // A8=0 controls the RAM enable flag.
    mbc.write(0x0000, 0x0A);
    mbc.write(0xA000, 0xAB);
    EXPECT_EQ(mbc.read(0xA000), 0xFB);
    EXPECT_EQ(mbc.read(0xA200), 0xFB);

    mbc.write(0x0000, 0x00);
    EXPECT_EQ(mbc.read(0xA000), 0xFF);

    mbc.reset();
    EXPECT_EQ(mbc.read(0xA000), 0xFF);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 1);
}

TEST(GbMbc2Test, SaveRamRoundTrip)
{
    auto romBanks = makeRomBanks(16);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks;
    qnemu::GbMbc2 mbc(std::move(romBanks), std::move(ramBanks), 0x06);

    mbc.write(0x0000, 0x0A);
    mbc.write(0xA000, 0xAB);
    EXPECT_TRUE(mbc.hasBattery());

    const std::vector<uint8_t> dump = mbc.dumpRam();
    ASSERT_EQ(dump.size(), 512U);
    EXPECT_EQ(dump.at(0), 0x0B);

    auto otherRomBanks = makeRomBanks(16);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> otherRamBanks;
    qnemu::GbMbc2 other(std::move(otherRomBanks), std::move(otherRamBanks), 0x06);
    other.loadRam(dump);
    other.write(0x0000, 0x0A);
    EXPECT_EQ(other.read(0xA000), 0xFB);
}

}  // namespace qnemuTest
