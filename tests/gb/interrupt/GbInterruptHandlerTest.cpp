/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

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

#include "mock/gb/cpu/MockGbCpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbInterruptTest, readAndWriteRegister)
{
    auto mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
    qnemu::GbInterruptHandler gbInterruptHandler(mockGbCpu);

    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0xE1);
    EXPECT_EQ(gbInterruptHandler.read(0xFFFF), 0);

    uint8_t value = distrib(gen);
    gbInterruptHandler.write(0xFF0F, value);
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), value);
    gbInterruptHandler.write(0xFFFF, value);
    EXPECT_EQ(gbInterruptHandler.read(0xFFFF), value);
}


}  // namespace qnemuTest