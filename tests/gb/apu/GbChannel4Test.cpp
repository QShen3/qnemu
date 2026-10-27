/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/apu/GbChannel4.h"

namespace qnemuTest
{

class GbChannel4Test : public testing::Test
{
public:
    void SetUp() override
    {
        channel = std::make_unique<qnemu::GbChannel4>();
    }

protected:
    std::unique_ptr<qnemu::GbChannel4> channel;
};

TEST_F(GbChannel4Test, ReadMasks)
{
    EXPECT_EQ(channel->read(0xFF20), 0xFF);
    EXPECT_EQ(channel->read(0xFF21), 0x00);
    EXPECT_EQ(channel->read(0xFF22), 0x00);
    EXPECT_EQ(channel->read(0xFF23), 0xBF);

    channel->write(0xFF21, 0xA5);
    EXPECT_EQ(channel->read(0xFF21), 0xA5);

    channel->write(0xFF22, 0x3C);
    EXPECT_EQ(channel->read(0xFF22), 0x3C);

    channel->write(0xFF23, 0xFF);
    EXPECT_EQ(channel->read(0xFF23), 0xFF);
}

TEST_F(GbChannel4Test, LfsrAdvancesAtConfiguredRate)
{
    channel->write(0xFF21, 0xF0);
    channel->write(0xFF22, 0x00);  // divider 0, shift 0 -> 8 cycles
    channel->write(0xFF23, 0x80);

    EXPECT_TRUE(channel->isEnabled());
    EXPECT_EQ(channel->getLfsr(), 0);

    for (int i = 0; i < 8; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getLfsr(), 0x4000);

    for (int i = 0; i < 8; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getLfsr(), 0x6000);
}

TEST_F(GbChannel4Test, NarrowLfsr)
{
    channel->write(0xFF21, 0xF0);
    channel->write(0xFF22, 0x08);  // 7-bit mode, divider 0
    channel->write(0xFF23, 0x80);

    for (int i = 0; i < 8; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getLfsr(), 0x4040);
}

TEST_F(GbChannel4Test, OutputFollowsLfsrBit)
{
    channel->write(0xFF21, 0xF0);
    channel->write(0xFF22, 0x00);
    channel->write(0xFF23, 0x80);

    // After 15 LFSR clocks the low bit becomes 1 and the channel outputs the
    // configured volume.
    for (int i = 0; i < 15 * 8; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getData(), 15);
}

TEST_F(GbChannel4Test, LengthTimerDisablesChannel)
{
    channel->write(0xFF20, 0x3F);  // length = 1
    channel->write(0xFF21, 0xF0);
    channel->write(0xFF23, 0xC0);  // length enable + trigger

    EXPECT_TRUE(channel->isEnabled());
    channel->clockLength();
    EXPECT_FALSE(channel->isEnabled());
}

}  // namespace qnemuTest
