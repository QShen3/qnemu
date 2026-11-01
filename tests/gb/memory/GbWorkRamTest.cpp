/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <random>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/CgbWorkRam.h"
#include "qnemu/gb/memory/DmgWorkRam.h"

namespace qnemuTest
{

namespace {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
}

TEST(CgbWorkRamTest, ReadAndWrite)
{
    qnemu::CgbWorkRam workRam;
    std::array<std::array<uint8_t, 0x1000>, 8> data;
    for (auto& bankData : data) {
        std::generate(bankData.begin(), bankData.end(), [&](){ return distrib(gen); });
    }

    for (uint16_t i = qnemu::WorkRamBank00Start; i < qnemu::WorkRamBank01Start; i++) {
        workRam.write(i, data.at(0).at(i - qnemu::WorkRamBank00Start));
    }
    for (uint8_t i = 1; i < 8; i++) {
        workRam.write(0xFF70, i);
        EXPECT_EQ(workRam.read(0xFF70), 0xF8 | i);
        for (uint16_t j = qnemu::WorkRamBank01Start; j <= qnemu::WorkRamBank01End; j++) {
            workRam.write(j, data.at(i).at(j - qnemu::WorkRamBank01Start));
        }
    }

    for (uint16_t i = qnemu::WorkRamBank00Start; i <= qnemu::WorkRamBank00End; i++) {
        EXPECT_EQ(workRam.read(i), data.at(0).at(i - qnemu::WorkRamBank00Start));
    }
    for (uint8_t i = 1; i < 8; i++) {
        workRam.write(0xFF70, i);
        for (uint16_t j = qnemu::WorkRamBank01Start; j <= qnemu::WorkRamBank01End; j++) {
            EXPECT_EQ(workRam.read(j), data.at(i).at(j - qnemu::WorkRamBank01Start));
        }
    }

    workRam.write(0xFF70, 0);
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        EXPECT_EQ(workRam.read(i), data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
    std::generate(data.at(1).begin(), data.at(1).end(), [&](){ return distrib(gen); });
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        workRam.write(i, data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        EXPECT_EQ(workRam.read(i), data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
}

TEST(CgbWorkRamTest, BankZeroDoesNotAliasSwitchableBank)
{
    qnemu::CgbWorkRam workRam;

    for (uint16_t address = qnemu::WorkRamBank00Start;
         address <= qnemu::WorkRamBank00End; ++address) {
        workRam.write(address, 0x11);
    }

    // SVBK=0 maps bank 1 at D000, and must not also write the fixed bank 0.
    workRam.write(0xFF70, 0x00);
    for (uint16_t address = qnemu::WorkRamBank01Start;
         address <= qnemu::WorkRamBank01End; ++address) {
        workRam.write(address, 0x22);
    }

    for (uint16_t address = qnemu::WorkRamBank00Start;
         address <= qnemu::WorkRamBank00End; ++address) {
        EXPECT_EQ(workRam.read(address), 0x11);
    }
    for (uint16_t address = qnemu::WorkRamBank01Start;
         address <= qnemu::WorkRamBank01End; ++address) {
        EXPECT_EQ(workRam.read(address), 0x22);
    }
}

TEST(CgbWorkRamTest, EchoRamMirrorsSelectedBanks)
{
    qnemu::CgbWorkRam workRam;

    workRam.write(0xC123, 0x5A);
    EXPECT_EQ(workRam.read(0xE123), 0x5A);
    workRam.write(0xE456, 0xA5);
    EXPECT_EQ(workRam.read(0xC456), 0xA5);

    workRam.write(0xFF70, 3);
    workRam.write(0xD321, 0x3C);
    EXPECT_EQ(workRam.read(0xF321), 0x3C);
    workRam.write(0xF654, 0xC3);
    EXPECT_EQ(workRam.read(0xD654), 0xC3);
}

TEST(CgbWorkRamTest, Reset)
{
    qnemu::CgbWorkRam workRam;
    EXPECT_EQ(workRam.read(0xFF70), 0xF8);

    const uint8_t value = distrib(gen);
    workRam.write(0xFF70, value);
    EXPECT_EQ(workRam.read(0xFF70), 0xF8 | (value & 0b111));

    workRam.reset();
    EXPECT_EQ(workRam.read(0xFF70), 0xF8);
}

TEST(CgbWorkRamTest, DmgCompatibilityUsesFixedUpperBank)
{
    qnemu::CgbWorkRam workRam;
    workRam.write(0xD000, 0x12);

    workRam.setDmgCompatibility(true);
    EXPECT_EQ(workRam.read(0xFF70), 0xFF);
    workRam.write(0xFF70, 5);
    EXPECT_EQ(workRam.read(0xD000), 0x12);
    workRam.write(0xD000, 0x34);
    EXPECT_EQ(workRam.read(0xD000), 0x34);

    workRam.setDmgCompatibility(false);
    workRam.write(0xFF70, 5);
    EXPECT_EQ(workRam.read(0xD000), 0x00);
    workRam.write(0xFF70, 1);
    EXPECT_EQ(workRam.read(0xD000), 0x34);
}

TEST(DmgWorkRamTest, IgnoresSvbkAndUsesFixedUpperBank)
{
    qnemu::DmgWorkRam workRam;

    EXPECT_EQ(workRam.read(0xFF70), 0xFF);
    workRam.write(0xFF70, 5);
    EXPECT_EQ(workRam.read(0xFF70), 0xFF);

    workRam.write(0xD000, 0x12);
    EXPECT_EQ(workRam.read(0xD000), 0x12);
    EXPECT_EQ(workRam.read(0xF000), 0x12);
}

TEST(DmgWorkRamTest, Step)
{
    qnemu::DmgWorkRam workRam;
    EXPECT_NO_THROW(workRam.step());
}

}  // namespace qnemuTest
