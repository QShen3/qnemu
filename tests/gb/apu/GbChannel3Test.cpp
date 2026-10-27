/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/apu/GbChannel3.h"

namespace qnemuTest
{

class GbChannel3Test : public testing::Test
{
public:
    void SetUp() override
    {
        channel = std::make_unique<qnemu::GbChannel3>();
    }

protected:
    void triggerWithWave(uint8_t sample)
    {
        channel->write(0xFF30, sample);
        channel->write(0xFF1A, 0x80);
        channel->write(0xFF1C, 0x20);
        channel->write(0xFF1D, 0xFF);
        channel->write(0xFF1E, 0x87);  // period 2047, trigger
    }

    std::unique_ptr<qnemu::GbChannel3> channel;
};

TEST_F(GbChannel3Test, ReadMasks)
{
    EXPECT_EQ(channel->read(0xFF1A), 0x7F);
    EXPECT_EQ(channel->read(0xFF1B), 0xFF);
    EXPECT_EQ(channel->read(0xFF1C), 0x9F);
    EXPECT_EQ(channel->read(0xFF1D), 0xFF);
    EXPECT_EQ(channel->read(0xFF1E), 0xBF);

    channel->write(0xFF1A, 0xFF);
    EXPECT_EQ(channel->read(0xFF1A), 0xFF);

    channel->write(0xFF1C, 0x60);
    EXPECT_EQ(channel->read(0xFF1C), 0xFF);

    channel->write(0xFF1E, 0xFF);
    EXPECT_EQ(channel->read(0xFF1E), 0xFF);
}

TEST_F(GbChannel3Test, WaveRamReadWriteWhenInactive)
{
    channel->write(0xFF30, 0x12);
    channel->write(0xFF3F, 0x34);
    EXPECT_EQ(channel->read(0xFF30), 0x12);
    EXPECT_EQ(channel->read(0xFF3F), 0x34);
}

TEST_F(GbChannel3Test, FirstSampleIsLowerNibble)
{
    triggerWithWave(0x12);

    EXPECT_TRUE(channel->isEnabled());
    EXPECT_TRUE(channel->isDacEnabled());

    channel->step();  // still emitting the previous (zero) sample
    EXPECT_EQ(channel->getData(), 0);

    channel->step();  // first wave RAM read: index 1 = lower nibble of FF30
    EXPECT_EQ(channel->getData(), 2);
}

TEST_F(GbChannel3Test, OutputLevels)
{
    // Fill the whole table so the output does not depend on which sample is
    // currently selected while the test changes NR32.
    for (uint16_t address = 0xFF30; address <= 0xFF3F; ++address) {
        channel->write(address, 0xFF);
    }
    triggerWithWave(0xFF);
    channel->step();
    channel->step();
    EXPECT_EQ(channel->getData(), 15);

    channel->write(0xFF1C, 0x40);  // 50%
    channel->step();
    EXPECT_EQ(channel->getData(), 7);

    channel->write(0xFF1C, 0x60);  // 25%
    channel->step();
    EXPECT_EQ(channel->getData(), 3);

    channel->write(0xFF1C, 0x00);  // mute
    channel->step();
    EXPECT_EQ(channel->getData(), 0);
}

TEST_F(GbChannel3Test, LengthTimerDisablesChannel)
{
    channel->write(0xFF1B, 0xFF);  // length = 1
    channel->write(0xFF1A, 0x80);
    channel->write(0xFF1E, 0xC0);  // length enable + trigger

    EXPECT_TRUE(channel->isEnabled());
    channel->clockLength();
    EXPECT_FALSE(channel->isEnabled());
}

TEST_F(GbChannel3Test, WaveRamSurvivesPowerOff)
{
    channel->write(0xFF30, 0x5A);
    channel->powerOff();

    EXPECT_EQ(channel->read(0xFF30), 0x5A);
    EXPECT_FALSE(channel->isEnabled());
}

}  // namespace qnemuTest
