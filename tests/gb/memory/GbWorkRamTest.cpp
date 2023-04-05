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

#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbWorkRamTest, ReadAndWrite)
{
    qnemu::GbWorkRam gbWorkRam;
    std::array<std::array<uint8_t, 0x1000>, 8> data;
    for (auto& bankData : data) {
        for (auto& value : bankData) {
            value = distrib(gen);
        }
    }

    for (uint16_t i = qnemu::WorkRamBank00Start; i < qnemu::WorkRamBank01Start; i++) {
        gbWorkRam.write(i, data.at(0).at(i - qnemu::WorkRamBank00Start));
    }
    for (uint8_t i = 1; i < 8; i++) {
        gbWorkRam.write(0xFF70, i);
        EXPECT_EQ(gbWorkRam.read(0xFF70), i);
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
    for (auto& value : data.at(1)) {
        value = distrib(gen);
    }
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        gbWorkRam.write(i, data.at(1).at(i - qnemu::WorkRamBank01Start));
    }
    for (uint16_t i = qnemu::WorkRamBank01Start; i <= qnemu::WorkRamBank01End; i++) {
        EXPECT_EQ(gbWorkRam.read(i), data.at(1).at(i - qnemu::WorkRamBank01Start));
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