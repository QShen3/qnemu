/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

#include <gtest/gtest.h>

#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "tests/support/FakeSound.h"

namespace qnemuTest
{

class GbApuTest : public testing::Test
{
public:
    void SetUp() override
    {
        auto sound = std::make_unique<FakeSound>();
        fakeSound = sound.get();

        std::array<std::unique_ptr<qnemu::GbChannelInterface>, 4> channels {
            std::make_unique<qnemu::GbChannel1>(),
            std::make_unique<qnemu::GbChannel2>(),
            std::make_unique<qnemu::GbChannel3>(),
            std::make_unique<qnemu::GbChannel4>(),
        };

        apu = std::make_unique<qnemu::GbApu>(std::move(sound), std::move(channels));
    }

    void setupWaveChannel()
    {
        for (uint16_t address = 0xFF30; address <= 0xFF3F; ++address) {
            apu->write(address, 0xFF);
        }
        apu->write(0xFF1A, 0x80);  // DAC on
        apu->write(0xFF1C, 0x20);  // 100% output level
        apu->write(0xFF1D, 0xFF);
        apu->write(0xFF1E, 0x87);  // period 2047, trigger
    }

protected:
    FakeSound* fakeSound = nullptr;
    std::unique_ptr<qnemu::GbApu> apu;
};

TEST_F(GbApuTest, PowerAndRegisterClearing)
{
    EXPECT_TRUE(apu->isPowered());
    EXPECT_EQ(apu->read(0xFF26), 0xF0);

    apu->write(0xFF16, 0x80);
    apu->write(0xFF17, 0xF0);
    apu->write(0xFF19, 0x80);
    EXPECT_EQ(apu->read(0xFF26), 0xF2);

    apu->write(0xFF26, 0x00);
    EXPECT_FALSE(apu->isPowered());
    EXPECT_EQ(apu->read(0xFF26), 0x70);
    EXPECT_EQ(apu->read(0xFF17), 0x00);

    apu->write(0xFF26, 0x80);
    EXPECT_TRUE(apu->isPowered());
    EXPECT_EQ(apu->read(0xFF26), 0xF0);
    EXPECT_EQ(apu->read(0xFF17), 0x00);
}

TEST_F(GbApuTest, WritesAreIgnoredWhilePoweredOff)
{
    apu->write(0xFF26, 0x00);
    apu->write(0xFF17, 0xF0);

    EXPECT_EQ(apu->read(0xFF17), 0x00);

    apu->write(0xFF26, 0x80);
    EXPECT_EQ(apu->read(0xFF17), 0x00);
}

TEST_F(GbApuTest, WaveRamSurvivesPowerOff)
{
    apu->write(0xFF30, 0x5A);
    apu->write(0xFF26, 0x00);

    EXPECT_EQ(apu->read(0xFF30), 0x5A);
}

TEST_F(GbApuTest, GlobalRegisterReadBack)
{
    apu->write(0xFF24, 0xFF);
    apu->write(0xFF25, 0xA5);

    EXPECT_EQ(apu->read(0xFF24), 0xFF);
    EXPECT_EQ(apu->read(0xFF25), 0xA5);
}

TEST_F(GbApuTest, DivApuClocksLength)
{
    apu->write(0xFF16, 0x3F);  // length = 1
    apu->write(0xFF17, 0xF0);
    apu->write(0xFF19, 0xC0);  // length enable + trigger

    EXPECT_EQ(apu->read(0xFF26) & 0x02, 0x02);

    for (int i = 0; i < 100; ++i) {
        apu->step();
    }
    EXPECT_EQ(apu->frameSequencerStep(), 0);
    EXPECT_EQ(apu->read(0xFF26) & 0x02, 0x02);

    apu->divApuTick();
    EXPECT_EQ(apu->frameSequencerStep(), 1);
    EXPECT_EQ(apu->read(0xFF26) & 0x02, 0x00);
}

TEST_F(GbApuTest, MixerPanningAndMasterVolume)
{
    setupWaveChannel();
    apu->write(0xFF24, 0x77);  // both master volumes at maximum
    apu->write(0xFF25, 0x04);  // CH3 to the right only

    apu->step();
    apu->step();

    EXPECT_EQ(apu->mixedLeft(), 0);
    EXPECT_EQ(apu->mixedRight(), -120);

    apu->write(0xFF25, 0x40);  // CH3 to the left only
    apu->step();
    EXPECT_EQ(apu->mixedLeft(), -120);
    EXPECT_EQ(apu->mixedRight(), 0);

    apu->write(0xFF25, 0x44);  // both
    apu->write(0xFF24, 0x33);  // master volume 3 + 1 = 4 on both sides
    apu->step();
    EXPECT_EQ(apu->mixedLeft(), -60);
    EXPECT_EQ(apu->mixedRight(), -60);
}

TEST_F(GbApuTest, MixerOutputIsInterleavedStereo)
{
    setupWaveChannel();
    apu->write(0xFF24, 0x77);
    apu->write(0xFF25, 0x44);

    for (int i = 0; i < 60000; ++i) {
        apu->step();
    }

    ASSERT_FALSE(fakeSound->frames.empty());
    EXPECT_LT(fakeSound->frames.front().first, -4000);
    EXPECT_LT(fakeSound->frames.front().second, -4000);
}

TEST_F(GbApuTest, AllDacsOffProducesSilence)
{
    for (int i = 0; i < 60000; ++i) {
        apu->step();
    }

    ASSERT_FALSE(fakeSound->frames.empty());
    EXPECT_EQ(fakeSound->frames.front().first, 0);
    EXPECT_EQ(fakeSound->frames.front().second, 0);
}

}  // namespace qnemuTest
