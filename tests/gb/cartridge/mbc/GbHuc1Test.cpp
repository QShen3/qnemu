/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/mbc/GbHuc1.h"
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

TEST(GbHuc1Test, RomRamAndIrMode)
{
    auto romBanks = makeRomBanks(8);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(4);
    qnemu::GbHuc1 mbc(std::move(romBanks), std::move(ramBanks), 0xFF);

    EXPECT_TRUE(mbc.hasBattery());

    mbc.write(qnemu::GbHuc1::romBankNumberAddress, 3);
    EXPECT_EQ(mbc.read(qnemu::MemoryRomBank01Start), 3);

    // RAM is always enabled in RAM mode; the 4000 register selects the bank.
    mbc.write(qnemu::GbHuc1::ramBankNumberAddress, 2);
    mbc.write(0xA123, 0x5A);
    EXPECT_EQ(mbc.read(0xA123), 0x5A);

    mbc.write(qnemu::GbHuc1::ramBankNumberAddress, 0);
    EXPECT_EQ(mbc.read(0xA123), 0x00);

    // 6000-7FFF is not connected and must not change the RAM bank.
    mbc.write(0x6000, 2);
    EXPECT_EQ(mbc.read(0xA123), 0x00);

    // 0x0E switches A000-BFFF to the IR register.  With no IR peer the
    // receiver reports "no light" (0xC0), and writes only set the local LED.
    mbc.write(qnemu::GbHuc1::irSelectAddress, 0x0E);
    EXPECT_EQ(mbc.read(0xA000), 0xC0);
    mbc.write(0xA000, 0x01);
    EXPECT_EQ(mbc.read(0xA000), 0xC0);

    mbc.write(qnemu::GbHuc1::irSelectAddress, 0x0A);
    mbc.write(qnemu::GbHuc1::ramBankNumberAddress, 2);
    EXPECT_EQ(mbc.read(0xA123), 0x5A);

    const std::vector<uint8_t> dump = mbc.dumpRam();
    EXPECT_EQ(dump.size(), 4 * qnemu::RamBankSize);

    auto otherRomBanks = makeRomBanks(8);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> otherRamBanks(4);
    qnemu::GbHuc1 other(std::move(otherRomBanks), std::move(otherRamBanks), 0xFF);
    other.loadRam(dump);
    EXPECT_EQ(other.dumpRam(), dump);
}

}  // namespace qnemuTest
