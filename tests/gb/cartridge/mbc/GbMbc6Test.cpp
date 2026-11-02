/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/mbc/GbMbc6.h"
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
        banks.at(bank).at(0x2000) = static_cast<uint8_t>(bank ^ 0x80);
    }
    return banks;
}

void selectFlashA(qnemu::GbMbc6& mbc, uint8_t bank)
{
    mbc.write(qnemu::GbMbc6::romBankASelectAddress, 0x08);
    mbc.write(qnemu::GbMbc6::flashEnableAddress, 0x01);
    mbc.write(qnemu::GbMbc6::romBankAAddress, bank);
}

void flashUnlock(qnemu::GbMbc6& mbc)
{
    mbc.write(qnemu::GbMbc6::romBankAAddress, 2);
    mbc.write(0x5555, 0xAA);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 1);
    mbc.write(0x4AAA, 0x55);
}

void flashCommand(qnemu::GbMbc6& mbc, uint8_t command)
{
    mbc.write(qnemu::GbMbc6::romBankAAddress, 2);
    mbc.write(0x5555, command);
}

void flashReset(qnemu::GbMbc6& mbc)
{
    mbc.write(0x4000, 0xF0);
}

}  // namespace

TEST(GbMbc6Test, DefaultRomWindowsAndBanking)
{
    qnemu::GbMbc6 mbc(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);

    // The fixed 16 KiB window always shows ROM bank 0.
    EXPECT_EQ(mbc.read(0x0000), 0x00);
    EXPECT_EQ(mbc.read(0x3FFF), 0x00);

    // Power-up presents the second 16 KiB ROM bank across 0x4000-0x7FFF.
    EXPECT_EQ(mbc.read(0x4000), 0x01);
    EXPECT_EQ(mbc.read(0x6000), 0x81);

    // The two windows bank independently.
    mbc.write(qnemu::GbMbc6::romBankAAddress, 4);
    mbc.write(qnemu::GbMbc6::romBankBAddress, 7);
    EXPECT_EQ(mbc.read(0x4000), 0x02);
    EXPECT_EQ(mbc.read(0x6000), 0x83);
    EXPECT_EQ(mbc.read(0x5FFF), 0x02);
}

TEST(GbMbc6Test, RamWindowsAndSaveData)
{
    qnemu::GbMbc6 mbc(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);

    EXPECT_TRUE(mbc.hasBattery());
    EXPECT_EQ(mbc.ramSize(), 0x8000);

    // RAM is disabled after reset.
    mbc.write(0xA000, 0x11);
    EXPECT_EQ(mbc.read(0xA000), 0xFF);

    mbc.write(qnemu::GbMbc6::ramEnableAddress, 0x0A);

    // 4 KiB RAM bank A 0 and 1 are the two halves of the first 8 KiB bank.
    mbc.write(qnemu::GbMbc6::ramBankAAddress, 0);
    mbc.write(0xA000, 0x11);
    EXPECT_EQ(mbc.read(0xA000), 0x11);

    mbc.write(qnemu::GbMbc6::ramBankAAddress, 1);
    EXPECT_EQ(mbc.read(0xA000), 0x00);
    mbc.write(0xA000, 0x22);
    mbc.write(qnemu::GbMbc6::ramBankAAddress, 0);
    EXPECT_EQ(mbc.read(0xA000), 0x11);

    // Window B has its own bank register.
    mbc.write(qnemu::GbMbc6::ramBankBAddress, 2);
    mbc.write(0xB000, 0x33);
    mbc.write(qnemu::GbMbc6::ramBankBAddress, 3);
    EXPECT_EQ(mbc.read(0xB000), 0x00);
    mbc.write(0xB000, 0x44);

    mbc.write(qnemu::GbMbc6::ramBankBAddress, 2);
    EXPECT_EQ(mbc.read(0xB000), 0x33);
    mbc.write(qnemu::GbMbc6::ramBankBAddress, 1);
    EXPECT_EQ(mbc.read(0xB000), 0x22);

    const std::vector<uint8_t> dump = mbc.dumpRam();
    EXPECT_EQ(dump.size(), 0x8000);

    qnemu::GbMbc6 restored(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);
    restored.loadRam(dump);
    EXPECT_EQ(restored.dumpRam(), dump);
}

TEST(GbMbc6Test, FlashIdMode)
{
    qnemu::GbMbc6 mbc(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);

    selectFlashA(mbc, 2);
    flashUnlock(mbc);
    flashCommand(mbc, 0x90);

    EXPECT_EQ(mbc.read(0x4000), 0xC2);
    EXPECT_EQ(mbc.read(0x4001), 0x81);
    EXPECT_EQ(mbc.read(0x4002), 0xC2);
    EXPECT_EQ(mbc.read(0x4003), 0xFF);

    flashReset(mbc);
    EXPECT_EQ(mbc.read(0x4000), 0xFF);
}

TEST(GbMbc6Test, FlashEraseProgramAndVerify)
{
    qnemu::GbMbc6 mbc(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);

    selectFlashA(mbc, 2);
    mbc.write(qnemu::GbMbc6::flashWriteEnableAddress, 0x01);

    // Erase sector 0.
    flashUnlock(mbc);
    flashCommand(mbc, 0x80);
    flashUnlock(mbc);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 0);
    mbc.write(0x4000, 0x30);
    EXPECT_EQ(mbc.read(0x4000), 0x80);
    flashReset(mbc);

    // Program one 128-byte page at flash address 0.
    flashUnlock(mbc);
    flashCommand(mbc, 0xA0);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 0);
    for (std::size_t i = 0; i < qnemu::GbMbc6::flashPageSize; ++i) {
        mbc.write(static_cast<uint16_t>(0x4000 + i), static_cast<uint8_t>(i));
    }
    mbc.write(0x407F, 0x00);  // Commit the page.
    flashReset(mbc);

    for (std::size_t i = 0; i < qnemu::GbMbc6::flashPageSize; ++i) {
        EXPECT_EQ(mbc.read(static_cast<uint16_t>(0x4000 + i)), static_cast<uint8_t>(i));
    }

    // Programming can only clear bits; 0xFF leaves the byte unchanged.
    flashUnlock(mbc);
    flashCommand(mbc, 0xA0);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 0);
    mbc.write(0x4000, 0xFF);
    mbc.write(0x4000, 0x00);  // Commit a single-byte page.
    flashReset(mbc);

    EXPECT_EQ(mbc.read(0x4000), 0x00);
}

TEST(GbMbc6Test, FlashWriteProtection)
{
    qnemu::GbMbc6 mbc(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);

    selectFlashA(mbc, 2);

    // Without /WP, sector 0 cannot be programmed.
    flashUnlock(mbc);
    flashCommand(mbc, 0xA0);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 0);
    mbc.write(0x4000, 0x00);
    mbc.write(0x4000, 0x00);
    flashReset(mbc);
    EXPECT_EQ(mbc.read(0x4000), 0xFF);

    // Sectors 1-7 are writable even while /WP is asserted.
    flashUnlock(mbc);
    flashCommand(mbc, 0xA0);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 16);  // Flash bank 16 = 0x20000.
    mbc.write(0x4000, 0x5A);
    mbc.write(0x4000, 0x00);
    flashReset(mbc);

    EXPECT_EQ(mbc.read(0x4000), 0x5A);
}

TEST(GbMbc6Test, Sector0ProtectionAndHiddenRegion)
{
    qnemu::GbMbc6 mbc(makeRomBanks(64),
        std::vector<std::array<uint8_t, qnemu::RamBankSize>>(4), 0x20);

    selectFlashA(mbc, 2);
    mbc.write(qnemu::GbMbc6::flashWriteEnableAddress, 0x01);

    // Protect sector 0.
    flashUnlock(mbc);
    flashCommand(mbc, 0x60);
    flashUnlock(mbc);
    mbc.write(0x4000, 0x20);
    EXPECT_EQ(mbc.read(0x4000), 0x82);
    flashReset(mbc);

    // Programming sector 0 is ignored while it is protected.
    flashUnlock(mbc);
    flashCommand(mbc, 0xA0);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 0);
    mbc.write(0x4000, 0x00);
    mbc.write(0x4000, 0x00);
    flashReset(mbc);
    EXPECT_EQ(mbc.read(0x4000), 0xFF);

    // Unprotect sector 0.
    flashUnlock(mbc);
    flashCommand(mbc, 0x60);
    flashUnlock(mbc);
    mbc.write(0x4000, 0x40);
    flashReset(mbc);

    // Program hidden map byte 0, then read it back through the 0x77 mode.
    flashUnlock(mbc);
    flashCommand(mbc, 0x60);
    flashUnlock(mbc);
    flashCommand(mbc, 0xE0);
    mbc.write(qnemu::GbMbc6::romBankAAddress, 0);
    mbc.write(0x4000, 0x12);
    mbc.write(0x4000, 0x00);
    flashReset(mbc);

    flashUnlock(mbc);
    flashCommand(mbc, 0x77);
    flashUnlock(mbc);
    flashCommand(mbc, 0x77);
    EXPECT_EQ(mbc.read(0x4000), 0x12);
    flashReset(mbc);
}

}  // namespace qnemuTest
