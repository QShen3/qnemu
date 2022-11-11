/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <gtest/gtest.h>

#include "mock/gb/interrupt/MockGbInterruptHandler.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemuTest
{

class GbTimerTest : public testing::Test
{
public:
    void SetUp() override
    {
        mockInterruptHandler = std::make_shared<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>>();
        gbTimer = std::make_unique<qnemu::GbTimer>(mockInterruptHandler);
    }
protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>> mockInterruptHandler;
    std::unique_ptr<qnemu::GbTimer> gbTimer;
};

TEST_F(GbTimerTest, Accepts)
{
    for (uint32_t i = 0; i <= 0xFFFF; i++) {
        if (i >= 0xFF04 && i <= 0xFF07) {
            EXPECT_TRUE(gbTimer->accepts(i));
        } else {
            EXPECT_FALSE(gbTimer->accepts(i));
        }
    }
}

TEST_F(GbTimerTest, Divider)
{
    EXPECT_EQ(gbTimer->read(0xFF04), 0xAB);
    gbTimer->write(0xFF04, 0xFF);
    EXPECT_EQ(gbTimer->read(0xFF04), 0);

    for (uint32_t i = 0; i < 0x100; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF04), 1);

    for (uint32_t i = 0; i < 0x100; i++) {
        for (uint32_t j = 0; j < 0x100; j++) {
            gbTimer->step();
        }
    }
    EXPECT_EQ(gbTimer->read(0xFF04), 1);
}

TEST_F(GbTimerTest, TimerNotEnable)
{
    // Timer is not enable
    gbTimer->write(0xFF07, 0);
    gbTimer->write(0xFF04, 0);

    for (uint32_t i = 0; i < 0x100; i++) {
        for (uint32_t j = 0; j < 0x100; j++) {
            gbTimer->step();
        }
    }
    EXPECT_EQ(gbTimer->read(0xFF04), 0);
    EXPECT_EQ(gbTimer->read(0xFF05), 0);
}

TEST_F(GbTimerTest, TimerCounter1024)
{
    // Enable Timer
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x04);
    gbTimer->write(0xFF06, 0x33);
    for (uint32_t i = 0; i < 1024; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
    gbTimer->write(0xFF04, 0);
    for (uint32_t i = 0; i < 0xFF; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            gbTimer->step();
        }
    }
    gbTimer->step();
    gbTimer->step();
    gbTimer->step();
    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    gbTimer->step();
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0x33);
}

TEST_F(GbTimerTest, TimerCounter16)
{
    // Enable Timer
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x05);
    gbTimer->write(0xFF06, 0x33);
    for (uint32_t i = 0; i < 16; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
    gbTimer->write(0xFF04, 0);
    for (uint32_t i = 0; i < 0xFF; i++) {
        for (uint32_t j = 0; j < 16; j++) {
            gbTimer->step();
        }
    }
    gbTimer->step();
    gbTimer->step();
    gbTimer->step();
    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    gbTimer->step();
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0x33);
}

TEST_F(GbTimerTest, TimerCounter64)
{
    // Enable Timer
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x06);
    gbTimer->write(0xFF06, 0x33);
    for (uint32_t i = 0; i < 64; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
    gbTimer->write(0xFF04, 0);
    for (uint32_t i = 0; i < 0xFF; i++) {
        for (uint32_t j = 0; j < 64; j++) {
            gbTimer->step();
        }
    }
    gbTimer->step();
    gbTimer->step();
    gbTimer->step();
    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    gbTimer->step();
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0x33);
}

TEST_F(GbTimerTest, TimerCounter256)
{
    // Enable Timer
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x07);
    gbTimer->write(0xFF06, 0x33);
    for (uint32_t i = 0; i < 256; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
    gbTimer->write(0xFF04, 0);
    for (uint32_t i = 0; i < 0xFF; i++) {
        for (uint32_t j = 0; j < 256; j++) {
            gbTimer->step();
        }
    }
    gbTimer->step();
    gbTimer->step();
    gbTimer->step();
    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    gbTimer->step();
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0x33);
}

TEST_F(GbTimerTest, Reset)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF05, 1);
    gbTimer->write(0xFF07, 0x07);
    gbTimer->write(0xFF06, 0x33);
    for (uint32_t i = 0; i < 256; i++) {
        gbTimer->step();
    }

    gbTimer->reset();
    EXPECT_EQ(gbTimer->read(0xFF04), 0xAB);
    EXPECT_EQ(gbTimer->read(0xFF05), 0);
    EXPECT_EQ(gbTimer->read(0xFF06), 0);
    EXPECT_EQ(gbTimer->read(0xFF07), 0xF8);
}

}  // namespace qnemuTest
