/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

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

#include "qnemu/gb/memory/GbHighRam.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbHighRamTest, Accepts)
{
    qnemu::GbHighRam gbHighRam;
    for (uint16_t i = 0; i < 0xFF80; i++) {
        EXPECT_FALSE(gbHighRam.accepts(i));
    }
    for (uint16_t i = 0xFF80; i < 0xFFFF; i++) {
        EXPECT_TRUE(gbHighRam.accepts(i));
    }
    EXPECT_FALSE(gbHighRam.accepts(0xFFFF));
}

TEST(GbHighRamTest, ReadAndWrite)
{
    qnemu::GbHighRam gbHighRam;
    std::array<uint8_t, 0x7F> data;
    for (auto& value : data) {
        value = distrib(gen);
    }

    for (uint16_t i = 0xFF80; i < 0xFFFF; i++) {
        gbHighRam.write(i, data.at(i - 0xFF80));
    }
    for (uint16_t i = 0xFF80; i < 0xFFFF; i++) {
        EXPECT_EQ(gbHighRam.read(i), data.at(i - 0xFF80));
    }
}

} // namespace qnemuTest
