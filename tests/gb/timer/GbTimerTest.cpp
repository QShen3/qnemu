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

// NOLINTBEGIN
protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>> mockInterruptHandler;
    std::unique_ptr<qnemu::GbTimer> gbTimer;
// NOLINTEND
};

TEST_F(GbTimerTest, ReadWriteError)
{
#ifdef NDEBUG
    EXPECT_EQ(gbTimer->read(0xFF08), 0xFF);
    EXPECT_NO_THROW(gbTimer->write(0xFF08, 0xFF));
#else
    EXPECT_DEATH(gbTimer->read(0xFF08), "");
    EXPECT_DEATH(gbTimer->write(0xFF08, 0), "");
#endif
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
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0x33);
}

TEST_F(GbTimerTest, DivApuTickUsesFallingEdge)
{
    // Power-on DIV is 0xAB00; bit 12 rises after 0x500 cycles and falls
    // after another 0x1000, which is the 512 Hz DIV-APU clock.
    for (uint32_t i = 0; i < 0x500; i++) {
        gbTimer->step();
    }
    EXPECT_FALSE(gbTimer->takeDivApuTick());

    for (uint32_t i = 0; i < 0x1000; i++) {
        gbTimer->step();
    }
    EXPECT_TRUE(gbTimer->takeDivApuTick());
    EXPECT_FALSE(gbTimer->takeDivApuTick());
}

TEST_F(GbTimerTest, DivWriteClocksApuWhenBitIsHigh)
{
    for (uint32_t i = 0; i < 0x500; i++) {
        gbTimer->step();
    }

    gbTimer->write(0xFF04, 0);
    EXPECT_TRUE(gbTimer->takeDivApuTick());
}

TEST_F(GbTimerTest, TimerUsesFallingEdge)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x04);  // enable, select bit 9

    for (uint32_t i = 0; i < 512; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 0);  // rising edge does nothing

    for (uint32_t i = 0; i < 512; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 1);  // falling edge increments
}

TEST_F(GbTimerTest, TacUpperBitsReadAsOne)
{
    gbTimer->write(0xFF07, 0x00);
    EXPECT_EQ(gbTimer->read(0xFF07), 0xF8);

    gbTimer->write(0xFF07, 0x07);
    EXPECT_EQ(gbTimer->read(0xFF07), 0xFF);
}

TEST_F(GbTimerTest, DivWriteCreatesTimerEdge)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x04);

    for (uint32_t i = 0; i < 512; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 0);

    gbTimer->write(0xFF04, 0);  // selected bit was high: falling edge
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
    EXPECT_EQ(gbTimer->read(0xFF04), 0);
}

TEST_F(GbTimerTest, TacClockSelectChangeCreatesTimerEdge)
{
    gbTimer->write(0xFF04, 0);
    for (uint32_t i = 0; i < 512; i++) {
        gbTimer->step();
    }

    gbTimer->write(0xFF07, 0x04);  // enable, bit 9 is currently high
    EXPECT_EQ(gbTimer->read(0xFF05), 0);

    gbTimer->write(0xFF07, 0x07);  // select bit 7, which is currently low
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
}

TEST_F(GbTimerTest, DisablingTimerCreatesTimerEdge)
{
    gbTimer->write(0xFF04, 0);
    for (uint32_t i = 0; i < 512; i++) {
        gbTimer->step();
    }
    gbTimer->write(0xFF07, 0x04);
    gbTimer->write(0xFF07, 0x00);  // disable while bit 9 is high
    EXPECT_EQ(gbTimer->read(0xFF05), 1);
}

TEST_F(GbTimerTest, OverflowStaysZeroForOneMcycleThenReloads)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x05);  // enable, 16-cycle clock
    gbTimer->write(0xFF06, 0x42);
    gbTimer->write(0xFF05, 0xFE);

    for (uint32_t i = 0; i < 32; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 0);

    gbTimer->step();
    gbTimer->step();
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0);

    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    gbTimer->step();
    EXPECT_EQ(gbTimer->read(0xFF05), 0x42);
    EXPECT_EQ(gbTimer->read(0xFF06), 0x42);
}

TEST_F(GbTimerTest, TimaWriteCancelsOverflowOnOverflowCycle)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x05);
    gbTimer->write(0xFF06, 0x42);
    gbTimer->write(0xFF05, 0xFE);

    for (uint32_t i = 0; i < 32; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 0);

    gbTimer->write(0xFF05, 0x7F);
    for (uint32_t i = 0; i < 15; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF05), 0x7F);
}

TEST_F(GbTimerTest, TimaWriteDuringReloadUsesTma)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x05);
    gbTimer->write(0xFF06, 0x42);
    gbTimer->write(0xFF05, 0xFE);

    for (uint32_t i = 0; i < 32; i++) {
        gbTimer->step();
    }

    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    for (uint32_t i = 0; i < 4; i++) {
        gbTimer->step();
    }

    gbTimer->write(0xFF05, 0x7F);
    EXPECT_EQ(gbTimer->read(0xFF05), 0x42);

    for (uint32_t i = 0; i < 4; i++) {
        gbTimer->step();
    }
    gbTimer->write(0xFF05, 0x7F);
    EXPECT_EQ(gbTimer->read(0xFF05), 0x7F);
}

TEST_F(GbTimerTest, TmaWriteDuringReloadCopiesToTima)
{
    gbTimer->write(0xFF04, 0);
    gbTimer->write(0xFF07, 0x05);
    gbTimer->write(0xFF06, 0x42);
    gbTimer->write(0xFF05, 0xFE);

    for (uint32_t i = 0; i < 32; i++) {
        gbTimer->step();
    }

    EXPECT_CALL(*mockInterruptHandler, requestTimerInterrupt());
    for (uint32_t i = 0; i < 4; i++) {
        gbTimer->step();
    }

    gbTimer->write(0xFF06, 0x55);
    EXPECT_EQ(gbTimer->read(0xFF06), 0x55);
    EXPECT_EQ(gbTimer->read(0xFF05), 0x55);
}

TEST_F(GbTimerTest, DoubleSpeedDoublesDividerRate)
{
    gbTimer->setDoubleSpeed(true);
    gbTimer->write(0xFF04, 0);

    for (uint32_t i = 0; i < 128; i++) {
        gbTimer->step();
    }
    EXPECT_EQ(gbTimer->read(0xFF04), 1);
}

TEST_F(GbTimerTest, DoubleSpeedUsesBit13ForDivApu)
{
    gbTimer->setDoubleSpeed(true);
    gbTimer->write(0xFF04, 0);

    for (uint32_t i = 0; i < 4096; i++) {
        gbTimer->step();
    }
    EXPECT_TRUE(gbTimer->takeDivApuTick());
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
