/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/apu/GbChannel1.h"

namespace qnemuTest
{

class GbChannel1Test : public testing::Test
{
public:
    void SetUp() override
    {
        channel = std::make_unique<qnemu::GbChannel1>();
    }

protected:
    std::unique_ptr<qnemu::GbChannel1> channel;
};

TEST_F(GbChannel1Test, ReadMasks)
{
    EXPECT_EQ(channel->read(0xFF10), 0x80);
    EXPECT_EQ(channel->read(0xFF11), 0x3F);
    EXPECT_EQ(channel->read(0xFF12), 0x00);
    EXPECT_EQ(channel->read(0xFF13), 0xFF);
    EXPECT_EQ(channel->read(0xFF14), 0xBF);

    channel->write(0xFF10, 0xFF);
    EXPECT_EQ(channel->read(0xFF10), 0xFF);

    channel->write(0xFF11, 0xFF);
    EXPECT_EQ(channel->read(0xFF11), 0xFF);

    channel->write(0xFF12, 0xAB);
    EXPECT_EQ(channel->read(0xFF12), 0xAB);

    channel->write(0xFF14, 0xFF);
    EXPECT_EQ(channel->read(0xFF14), 0xFF);
}

TEST_F(GbChannel1Test, TriggerAndDutyOutput)
{
    channel->write(0xFF11, 0x80);  // 50% duty
    channel->write(0xFF12, 0xF0);  // volume 15, DAC on, envelope off
    channel->write(0xFF13, 0x00);
    channel->write(0xFF14, 0x80);  // trigger, period 0

    EXPECT_TRUE(channel->isEnabled());
    EXPECT_TRUE(channel->isDacEnabled());
    EXPECT_EQ(channel->getPeriod(), 0);

    channel->step();
    EXPECT_EQ(channel->getData(), 15);

    // Period 0 -> 8192 cycles per duty step.
    for (int i = 0; i < 8191; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getData(), 0);

    // Next high part of the 50% pattern is at position 5.
    for (int i = 0; i < 4 * 8192; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getData(), 15);
}

TEST_F(GbChannel1Test, EnvelopeChangesVolume)
{
    channel->write(0xFF11, 0x80);
    channel->write(0xFF12, 0x0A);  // initial 0, increase, pace 2
    channel->write(0xFF13, 0x00);
    channel->write(0xFF14, 0x80);

    channel->step();
    EXPECT_EQ(channel->getData(), 0);

    channel->clockEnvelope();
    channel->clockEnvelope();
    channel->step();
    EXPECT_EQ(channel->getData(), 1);

    channel->clockEnvelope();
    channel->clockEnvelope();
    channel->step();
    EXPECT_EQ(channel->getData(), 2);
}

TEST_F(GbChannel1Test, LengthTimerDisablesChannel)
{
    channel->write(0xFF11, 0x3F);  // length = 1
    channel->write(0xFF12, 0xF0);
    channel->write(0xFF14, 0xC0);  // length enable + trigger

    EXPECT_TRUE(channel->isEnabled());
    channel->clockLength();
    EXPECT_FALSE(channel->isEnabled());
    EXPECT_EQ(channel->getData(), 0);
}

TEST_F(GbChannel1Test, DacOffDisablesChannel)
{
    channel->write(0xFF12, 0xF0);
    channel->write(0xFF14, 0x80);
    EXPECT_TRUE(channel->isEnabled());

    channel->write(0xFF12, 0x00);
    EXPECT_FALSE(channel->isDacEnabled());
    EXPECT_FALSE(channel->isEnabled());
    EXPECT_EQ(channel->getData(), 0);
}

TEST_F(GbChannel1Test, SweepIncreasesPeriod)
{
    channel->write(0xFF10, 0x11);  // pace 1, add, step 1
    channel->write(0xFF11, 0x80);
    channel->write(0xFF12, 0xF0);
    channel->write(0xFF13, 0x00);
    channel->write(0xFF14, 0x82);  // period high 2 -> 512

    EXPECT_TRUE(channel->isEnabled());
    EXPECT_EQ(channel->getPeriod(), 512);

    channel->clockSweep();
    EXPECT_EQ(channel->getPeriod(), 768);
    EXPECT_TRUE(channel->isEnabled());

    channel->clockSweep();
    EXPECT_EQ(channel->getPeriod(), 1152);
    EXPECT_TRUE(channel->isEnabled());

    channel->clockSweep();
    EXPECT_EQ(channel->getPeriod(), 1728);
    EXPECT_FALSE(channel->isEnabled());
}

TEST_F(GbChannel1Test, SweepOverflowOnTriggerDisablesChannel)
{
    channel->write(0xFF10, 0x11);
    channel->write(0xFF12, 0xF0);
    channel->write(0xFF13, 0xFF);
    channel->write(0xFF14, 0x87);  // period 2047

    EXPECT_FALSE(channel->isEnabled());
}

TEST_F(GbChannel1Test, SweepDecreasesPeriod)
{
    channel->write(0xFF10, 0x1A);  // pace 1, subtract, step 2
    channel->write(0xFF12, 0xF0);
    channel->write(0xFF13, 0x00);
    channel->write(0xFF14, 0x84);  // period 1024

    EXPECT_TRUE(channel->isEnabled());
    EXPECT_EQ(channel->getPeriod(), 1024);

    channel->clockSweep();
    EXPECT_EQ(channel->getPeriod(), 768);
    EXPECT_TRUE(channel->isEnabled());

    channel->clockSweep();
    EXPECT_EQ(channel->getPeriod(), 576);
    EXPECT_TRUE(channel->isEnabled());
}

}  // namespace qnemuTest
