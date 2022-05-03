/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <random>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbWorkRamTest, Accepts)
{
    qnemu::GbWorkRam gbWorkRam;
    for (uint16_t i = 0; i < 0xC000; i++) {
        EXPECT_FALSE(gbWorkRam.accepts(i));
    }
    for (uint16_t i = 0xC000; i < 0xFE00; i++) {
        EXPECT_TRUE(gbWorkRam.accepts(i));
    }
    for (uint32_t i = 0xFE00; i < 0x10000; i++) {
        if (i == 0xFF70) {
            EXPECT_TRUE(gbWorkRam.accepts(i));
            continue;
        }
        EXPECT_FALSE(gbWorkRam.accepts(i));
    }
}

TEST(GbWorkRamTest, ReadAndWrite)
{
    qnemu::GbWorkRam gbWorkRam;
    std::array<std::array<uint8_t, 0x1000>, 8> data;
    for (auto& bankData : data) {
        for (auto& value : bankData) {
            value = distrib(gen);
        }
    }

    for (uint16_t i = 0xC000; i < 0xD000; i++) {
        gbWorkRam.write(i, data.at(0).at(i - 0xC000));
    }
    for (uint8_t i = 1; i < 8; i++) {
        gbWorkRam.write(0xFF70, i);
        EXPECT_EQ(gbWorkRam.read(0xFF70), i);
        for (uint16_t j = 0xD000; j < 0xE000; j++) {
            gbWorkRam.write(j, data.at(i).at(j - 0xD000));
        }
    }

    for (uint16_t i = 0xC000; i < 0xD000; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(0).at(i - 0xC000));
    }
    for (uint8_t i = 1; i < 8; i++) {
        gbWorkRam.write(0xFF70, i);
        for (uint16_t j = 0xD000; j < 0xE000; j++) {
            EXPECT_EQ(gbWorkRam.read(j), data.at(i).at(j - 0xD000));
        }
    }

    gbWorkRam.write(0xFF70, 0);
    for (uint16_t i = 0xD000; i < 0xE000; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(1).at(i - 0xD000));
    }
    for (auto& value : data.at(1)) {
        value = distrib(gen);
    }
    for (uint16_t i = 0xD000; i < 0xE000; i++) {
        gbWorkRam.write(i, data.at(1).at(i - 0xD000));
    }
    for (uint16_t i = 0xD000; i < 0xE000; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(1).at(i - 0xD000));
    }
}

TEST(GbWorkRamTest, Reset)
{
    qnemu::GbWorkRam gbWorkRam;
    EXPECT_EQ(gbWorkRam.read(0xFF70), 1);

    uint8_t value = distrib(gen);
    gbWorkRam.write(0xFF70, value);
    EXPECT_EQ(gbWorkRam.read(0xFF70), value & 0b111);

    gbWorkRam.reset();
    EXPECT_EQ(gbWorkRam.read(0xFF70), 1);
}

}  // namespace qnemuTest