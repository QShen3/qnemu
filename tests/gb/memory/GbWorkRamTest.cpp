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
#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemuTest
{

namespace {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
}

TEST(GbWorkRamTest, ReadAndWrite)
{
    qnemu::GbWorkRam gbWorkRam;
    std::array<std::array<uint8_t, 0x1000>, 8> data;
    for (auto& bankData : data) {
        std::generate(bankData.begin(), bankData.end(), [&](){ return distrib(gen); });
    }

    for (uint16_t i = qnemu::WorkRamBank00Start; i < qnemu::WorkRamBank01Start; i++) {
        gbWorkRam.write(i, data.at(0).at(i - qnemu::WorkRamBank00Start));
    }
    for (uint8_t i = 1; i < 8; i++) {
        gbWorkRam.write(0xFF70, i);
        EXPECT_EQ(gbWorkRam.read(0xFF70), 0xF8 | i);
        for (uint16_t j = qnemu::WorkRamBank01Start; j <= qnemu::WorkRamBank01End; j++) {
            gbWorkRam.write(j, data.at(i).at(j - qnemu::WorkRamBank01Start));
        }
    }

    for (uint16_t i = qnemu::WorkRamBank00Start; i <= qnemu::WorkRamBank00End; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(0).at(i - qnemu::WorkRamBank00Start));
    }
    for (uint8_t i = 1; i < 8; i++) {
        gbWorkRam.write(0xFF70, i);
        for (uint16_t j = qnemu::WorkRamBank01Start; j <= qnemu::WorkRamBank01End; j++) {
            EXPECT_EQ(gbWorkRam.read(j), data.at(i).at(j - qnemu::WorkRamBank01Start));
        }
    }

    gbWorkRam.write(0xFF70, 0);
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
    std::generate(data.at(1).begin(), data.at(1).end(), [&](){ return distrib(gen); });
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        gbWorkRam.write(i, data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
}

TEST(GbWorkRamTest, BankZeroDoesNotAliasSwitchableBank)
{
    qnemu::GbWorkRam gbWorkRam;

    for (uint16_t address = qnemu::WorkRamBank00Start;
         address <= qnemu::WorkRamBank00End; ++address) {
        gbWorkRam.write(address, 0x11);
    }

    // SVBK=0 maps bank 1 at D000, and must not also write the fixed bank 0.
    gbWorkRam.write(0xFF70, 0x00);
    for (uint16_t address = qnemu::WorkRamBank01Start;
         address <= qnemu::WorkRamBank01End; ++address) {
        gbWorkRam.write(address, 0x22);
    }

    for (uint16_t address = qnemu::WorkRamBank00Start;
         address <= qnemu::WorkRamBank00End; ++address) {
        EXPECT_EQ(gbWorkRam.read(address), 0x11);
    }
    for (uint16_t address = qnemu::WorkRamBank01Start;
         address <= qnemu::WorkRamBank01End; ++address) {
        EXPECT_EQ(gbWorkRam.read(address), 0x22);
    }
}

TEST(GbWorkRamTest, EchoRamMirrorsSelectedBanks)
{
    qnemu::GbWorkRam gbWorkRam;

    gbWorkRam.write(0xC123, 0x5A);
    EXPECT_EQ(gbWorkRam.read(0xE123), 0x5A);
    gbWorkRam.write(0xE456, 0xA5);
    EXPECT_EQ(gbWorkRam.read(0xC456), 0xA5);

    gbWorkRam.write(0xFF70, 3);
    gbWorkRam.write(0xD321, 0x3C);
    EXPECT_EQ(gbWorkRam.read(0xF321), 0x3C);
    gbWorkRam.write(0xF654, 0xC3);
    EXPECT_EQ(gbWorkRam.read(0xD654), 0xC3);
}

TEST(GbWorkRamTest, Reset)
{
    qnemu::GbWorkRam gbWorkRam;
    EXPECT_EQ(gbWorkRam.read(0xFF70), 0xF8);

    const uint8_t value = distrib(gen);
    gbWorkRam.write(0xFF70, value);
    EXPECT_EQ(gbWorkRam.read(0xFF70), 0xF8 | (value & 0b111));

    gbWorkRam.reset();
    EXPECT_EQ(gbWorkRam.read(0xFF70), 0xF8);
}

TEST(GbWorkRamTest, DmgIgnoresSvbkAndUsesFixedUpperBank)
{
    qnemu::GbWorkRam gbWorkRam;
    gbWorkRam.setCgb(false);

    EXPECT_EQ(gbWorkRam.read(0xFF70), 0xFF);
    gbWorkRam.write(0xFF70, 5);
    EXPECT_EQ(gbWorkRam.read(0xFF70), 0xFF);

    // The DMG's D000-DFFF is the fixed upper 4 KiB, i.e. CGB bank 1.
    gbWorkRam.write(0xD000, 0x12);
    gbWorkRam.setCgb(true);
    gbWorkRam.write(0xFF70, 1);
    EXPECT_EQ(gbWorkRam.read(0xD000), 0x12);

    gbWorkRam.write(0xFF70, 5);
    EXPECT_EQ(gbWorkRam.read(0xD000), 0x00);
}

TEST(GbWorkRamTest, Step)
{
    qnemu::GbWorkRam gbWorkRam;
    EXPECT_NO_THROW(gbWorkRam.step());
}

}  // namespace qnemuTest
