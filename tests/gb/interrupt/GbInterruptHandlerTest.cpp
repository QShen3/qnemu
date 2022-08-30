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
#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbInterruptTest, ReadAndWriteRegister)
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

TEST(GbInterruptTest, Reset)
{
    auto mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
    qnemu::GbInterruptHandler gbInterruptHandler(mockGbCpu);

    uint8_t value = distrib(gen);
    gbInterruptHandler.write(0xFF0F, value);
    value = distrib(gen);
    gbInterruptHandler.write(0xFFFF, value);

    gbInterruptHandler.reset();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0xE1);
    EXPECT_EQ(gbInterruptHandler.read(0xFFFF), 0);
}

TEST(GbInterruptTest, Step)
{
    auto mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
    qnemu::GbInterruptHandler gbInterruptHandler(mockGbCpu);

    qnemu::GbDeviceInterface::interruptMasterEnabled = false;
    gbInterruptHandler.step();

    qnemu::GbDeviceInterface::interruptMasterEnabled = true;
    gbInterruptHandler.write(0xFFFF, 0);
    gbInterruptHandler.step();
    gbInterruptHandler.write(0xFFFF, 0xFF);
    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.step();

    for (uint8_t i = 0; i < 5; i++) {
        qnemu::GbDeviceInterface::interruptMasterEnabled = true;
        gbInterruptHandler.write(0xFFFF, 1 << i);
        gbInterruptHandler.write(0xFF0F, 1 << i);
        EXPECT_CALL(*mockGbCpu, jumpToAddress(0x40 + i * 8));
        gbInterruptHandler.step();
        EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0);
        EXPECT_EQ(qnemu::GbDeviceInterface::interruptMasterEnabled, false);
    }
}

TEST(GbInterruptTest, RequesetInterrupt)
{
    auto mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
    qnemu::GbInterruptHandler gbInterruptHandler(mockGbCpu);

    EXPECT_CALL(*mockGbCpu, isInHaltMode()).Times(5).WillRepeatedly(testing::Return(false));
    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestVBlankInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b1);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestLcdInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b10);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestTimerInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b100);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestSerialInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b1000);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestJoypadInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b10000);


    EXPECT_CALL(*mockGbCpu, isInHaltMode()).Times(5).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*mockGbCpu, exitHaltMode()).Times(5);
    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestVBlankInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b1);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestLcdInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b10);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestTimerInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b100);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestSerialInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b1000);

    gbInterruptHandler.write(0xFF0F, 0);
    gbInterruptHandler.requestJoypadInterrupt();
    EXPECT_EQ(gbInterruptHandler.read(0xFF0F), 0b10000);
}

}  // namespace qnemuTest