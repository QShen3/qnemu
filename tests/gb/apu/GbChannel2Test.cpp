/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/apu/GbChannel2.h"

namespace qnemuTest
{

class GbChannel2Test : public testing::Test
{
public:
    void SetUp() override
    {
        channel = std::make_unique<qnemu::GbChannel2>();
    }

protected:
    std::unique_ptr<qnemu::GbChannel2> channel;
};

TEST_F(GbChannel2Test, ReadMasks)
{
    EXPECT_EQ(channel->read(0xFF16), 0x3F);
    EXPECT_EQ(channel->read(0xFF17), 0x00);
    EXPECT_EQ(channel->read(0xFF18), 0xFF);
    EXPECT_EQ(channel->read(0xFF19), 0xBF);

    channel->write(0xFF16, 0xFF);
    EXPECT_EQ(channel->read(0xFF16), 0xFF);

    channel->write(0xFF17, 0xA5);
    EXPECT_EQ(channel->read(0xFF17), 0xA5);

    channel->write(0xFF19, 0xFF);
    EXPECT_EQ(channel->read(0xFF19), 0xFF);
}

TEST_F(GbChannel2Test, DutyOutput)
{
    channel->write(0xFF16, 0x00);  // 12.5% duty
    channel->write(0xFF17, 0xF0);
    channel->write(0xFF18, 0x00);
    channel->write(0xFF19, 0x80);

    EXPECT_TRUE(channel->isEnabled());
    channel->step();
    EXPECT_EQ(channel->getData(), 0);

    // The 12.5% pattern is high at position 7 only.
    for (int i = 0; i < 7 * 8192; ++i) {
        channel->step();
    }
    EXPECT_EQ(channel->getData(), 15);
}

TEST_F(GbChannel2Test, LengthTimerDisablesChannel)
{
    channel->write(0xFF16, 0x3F);
    channel->write(0xFF17, 0xF0);
    channel->write(0xFF19, 0xC0);

    EXPECT_TRUE(channel->isEnabled());
    channel->clockLength();
    EXPECT_FALSE(channel->isEnabled());
}

TEST_F(GbChannel2Test, TriggerNeedsEnabledDac)
{
    channel->write(0xFF17, 0x00);  // DAC off
    channel->write(0xFF19, 0x80);
    EXPECT_FALSE(channel->isEnabled());

    channel->write(0xFF17, 0xF0);
    channel->write(0xFF19, 0x80);
    EXPECT_TRUE(channel->isEnabled());
}

}  // namespace qnemuTest
