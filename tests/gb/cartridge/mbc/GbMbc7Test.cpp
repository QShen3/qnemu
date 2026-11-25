/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/mbc/GbAccelerometerState.h"
#include "qnemu/gb/cartridge/mbc/GbMbc7.h"
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

std::shared_ptr<qnemu::AccelerometerState> makeAccelerometer()
{
    return std::make_shared<qnemu::AccelerometerState>();
}

void enableRegisters(qnemu::GbMbc7& mbc)
{
    mbc.write(qnemu::GbMbc7::ramEnableAddress, 0x0A);
    mbc.write(qnemu::GbMbc7::ramEnable2Address, 0x40);
}

void eepromPins(qnemu::GbMbc7& mbc, bool cs, bool clock, bool dataIn)
{
    mbc.write(0xA080, static_cast<uint8_t>((cs ? 0x80 : 0x00)
        | (clock ? 0x40 : 0x00) | (dataIn ? 0x02 : 0x00)));
}

void eepromCommand(qnemu::GbMbc7& mbc, uint16_t command)
{
    eepromPins(mbc, false, false, false);
    eepromPins(mbc, true, false, false);
    for (int bit = 9; bit >= 0; --bit) {
        const bool value = ((command >> bit) & 1) != 0;
        eepromPins(mbc, true, false, value);
        eepromPins(mbc, true, true, value);
    }
}

uint16_t eepromRead(qnemu::GbMbc7& mbc, uint8_t address)
{
    eepromCommand(mbc, static_cast<uint16_t>(0x300 | (address << 1)));
    uint16_t value = 0;
    for (int bit = 0; bit < 16; ++bit) {
        eepromPins(mbc, true, false, false);
        eepromPins(mbc, true, true, false);
        value = static_cast<uint16_t>((value << 1) | (mbc.read(0xA080) & 1));
    }
    eepromPins(mbc, false, false, false);
    return value;
}

void eepromWrite(qnemu::GbMbc7& mbc, uint8_t address, uint16_t value)
{
    eepromCommand(mbc, static_cast<uint16_t>(0x280 | (address << 1)));
    for (int bit = 15; bit >= 0; --bit) {
        const bool data = ((value >> bit) & 1) != 0;
        eepromPins(mbc, true, false, data);
        eepromPins(mbc, true, true, data);
    }
    eepromPins(mbc, false, false, false);
}

}  // namespace

TEST(GbMbc7Test, RomBankingAndTwoStageRegisterEnable)
{
    qnemu::GbMbc7 mbc(makeRomBanks(4), {}, 0x22, makeAccelerometer());

    EXPECT_EQ(mbc.read(0x0000), 0x00);
    EXPECT_EQ(mbc.read(0x4000), 0x01);
    mbc.write(qnemu::GbMbc7::romBankAddress, 0x03);
    EXPECT_EQ(mbc.read(0x4000), 0x03);

    EXPECT_EQ(mbc.read(0xA020), 0xFF);
    mbc.write(qnemu::GbMbc7::ramEnableAddress, 0x0A);
    EXPECT_EQ(mbc.read(0xA020), 0xFF);
    mbc.write(qnemu::GbMbc7::ramEnable2Address, 0x40);
    EXPECT_EQ(mbc.read(0xA020), 0x00);
    EXPECT_EQ(mbc.read(0xA030), 0x80);
    EXPECT_EQ(mbc.read(0xB000), 0xFF);

    mbc.write(qnemu::GbMbc7::ramEnable2Address, 0x00);
    EXPECT_EQ(mbc.read(0xA020), 0xFF);
}

TEST(GbMbc7Test, AccelerometerRequiresEraseBeforeLatch)
{
    auto accelerometer = makeAccelerometer();
    qnemu::GbMbc7 mbc(makeRomBanks(4), {}, 0x22, accelerometer);
    enableRegisters(mbc);

    accelerometer->set(0.5F, -0.5F);
    mbc.write(0xA010, 0xAA);
    EXPECT_EQ(mbc.read(0xA020), 0x00);
    EXPECT_EQ(mbc.read(0xA030), 0x80);

    mbc.write(0xA000, 0x55);
    EXPECT_EQ(mbc.read(0xA020), 0x00);
    EXPECT_EQ(mbc.read(0xA030), 0x80);
    mbc.write(0xA010, 0xAA);
    EXPECT_EQ(mbc.read(0xA020), 0x08);
    EXPECT_EQ(mbc.read(0xA030), 0x82);
    EXPECT_EQ(mbc.read(0xA040), 0x98);
    EXPECT_EQ(mbc.read(0xA050), 0x81);
    EXPECT_EQ(mbc.read(0xA060), 0x00);
    EXPECT_EQ(mbc.read(0xA070), 0xFF);

    accelerometer->set(-1.0F, 1.0F);
    mbc.write(0xA010, 0xAA);
    EXPECT_EQ(mbc.read(0xA030), 0x82);
    mbc.write(0xA000, 0x55);
    mbc.write(0xA010, 0xAA);
    EXPECT_EQ(mbc.read(0xA020), 0x60);
    EXPECT_EQ(mbc.read(0xA030), 0x81);
    EXPECT_EQ(mbc.read(0xA040), 0x40);
    EXPECT_EQ(mbc.read(0xA050), 0x82);
}

TEST(GbMbc7Test, EepromReadWriteEraseAndEnable)
{
    qnemu::GbMbc7 mbc(makeRomBanks(4), {}, 0x22, makeAccelerometer());
    enableRegisters(mbc);

    EXPECT_TRUE(mbc.hasBattery());
    EXPECT_EQ(mbc.ramSize(), 0x100);
    EXPECT_EQ(eepromRead(mbc, 0), 0xFFFF);

    // Writes are locked until EWEN.
    eepromWrite(mbc, 0, 0x1234);
    EXPECT_EQ(eepromRead(mbc, 0), 0xFFFF);

    eepromCommand(mbc, 0x260);  // EWEN
    eepromWrite(mbc, 0, 0x1234);
    eepromWrite(mbc, 1, 0xABCD);
    EXPECT_EQ(mbc.dumpRam().at(2), 0xCD);
    EXPECT_EQ(mbc.dumpRam().at(3), 0xAB);
    EXPECT_EQ(eepromRead(mbc, 0), 0x1234);
    EXPECT_EQ(mbc.dumpRam().at(2), 0xCD);
    EXPECT_EQ(mbc.dumpRam().at(3), 0xAB);
    EXPECT_EQ(eepromRead(mbc, 1), 0xABCD);
    EXPECT_EQ(eepromRead(mbc, 1), 0xABCD);
    EXPECT_EQ(eepromRead(mbc, 0), 0x1234);

    eepromCommand(mbc, 0x380);  // ERASE word 0
    EXPECT_EQ(eepromRead(mbc, 0), 0xFFFF);
    EXPECT_EQ(eepromRead(mbc, 1), 0xABCD);

    eepromCommand(mbc, 0x200);  // EWDS
    eepromWrite(mbc, 1, 0x0000);
    EXPECT_EQ(eepromRead(mbc, 1), 0xABCD);
}

TEST(GbMbc7Test, EepromSaveRoundTripSurvivesReset)
{
    qnemu::GbMbc7 mbc(makeRomBanks(4), {}, 0x22, makeAccelerometer());
    enableRegisters(mbc);
    eepromCommand(mbc, 0x260);
    eepromWrite(mbc, 0x20, 0x55AA);

    const std::vector<uint8_t> dump = mbc.dumpRam();
    ASSERT_EQ(dump.size(), 0x100);
    EXPECT_EQ(dump.at(0x40), 0xAA);
    EXPECT_EQ(dump.at(0x41), 0x55);

    qnemu::GbMbc7 restored(makeRomBanks(4), {}, 0x22, makeAccelerometer());
    restored.loadRam(dump);
    restored.reset();
    enableRegisters(restored);
    EXPECT_EQ(eepromRead(restored, 0x20), 0x55AA);
}

}  // namespace qnemuTest
