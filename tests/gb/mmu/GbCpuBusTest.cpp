/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "qnemu/gb/mmu/GbCpuBus.h"

namespace qnemuTest
{

namespace
{
class FakeMmu : public qnemu::GbMmuInterface
{
public:
    uint8_t read(uint16_t) const override { return 0; }
    void write(uint16_t, const uint8_t&) override {}
    void step() override { ++steps; }
    void reset() override {}
    void setDoubleSpeed(bool value) override { doubleSpeed = value; }
    void setCgb(bool value) override { cgb = value; }
    void setDmgCompatibilityMode(bool value) override { dmgMode = value; }

    int steps = 0;
    bool doubleSpeed = false;
    bool cgb = false;
    bool dmgMode = false;
};
}  // namespace

TEST(GbCpuBusTest, CgbKeyRegistersAndSpeedSwitch)
{
    FakeMmu mmu;
    qnemu::GbCpuBus bus(mmu, true);

    EXPECT_EQ(bus.read(0xFF4C), 0x80);
    EXPECT_EQ(bus.read(0xFF4D), 0x7E);

    bus.write(0xFF4D, 0x01);
    EXPECT_EQ(bus.read(0xFF4D), 0x7F);
    EXPECT_FALSE(bus.onStop());
    EXPECT_TRUE(mmu.doubleSpeed);
    EXPECT_EQ(bus.read(0xFF4D), 0xFE);

    bus.tick(4);
    EXPECT_EQ(mmu.steps, 2);
    bus.tick(1);
    EXPECT_EQ(mmu.steps, 2);
    bus.tick(1);
    EXPECT_EQ(mmu.steps, 3);
}

TEST(GbCpuBusTest, DmgDoesNotExposeKeyRegisters)
{
    FakeMmu mmu;
    qnemu::GbCpuBus bus(mmu, false);

    EXPECT_EQ(bus.read(0xFF4C), 0xFF);
    EXPECT_EQ(bus.read(0xFF4D), 0x7E);
    bus.write(0xFF4D, 0x01);
    EXPECT_TRUE(bus.onStop());
    EXPECT_EQ(bus.read(0xFF4D), 0x7E);
}

TEST(GbCpuBusTest, ResetReturnsToNormalSpeed)
{
    FakeMmu mmu;
    qnemu::GbCpuBus bus(mmu, true);

    bus.write(0xFF4D, 0x01);
    ASSERT_FALSE(bus.onStop());
    ASSERT_TRUE(mmu.doubleSpeed);
    ASSERT_EQ(bus.read(0xFF4D), 0xFE);

    bus.reset();
    EXPECT_FALSE(mmu.doubleSpeed);
    EXPECT_EQ(bus.read(0xFF4D), 0x7E);

    bus.tick(4);
    EXPECT_EQ(mmu.steps, 4);
}

TEST(GbCpuBusTest, SetCgbForwardsModelToMmu)
{
    FakeMmu mmu;
    qnemu::GbCpuBus bus(mmu, false);

    bus.setCgb(true);
    EXPECT_TRUE(mmu.cgb);
    bus.setCgb(false);
    EXPECT_FALSE(mmu.cgb);
}

TEST(GbCpuBusTest, Key0SwitchesToDmgCompatibility)
{
    FakeMmu mmu;
    qnemu::GbCpuBus bus(mmu, true);

    bus.write(0xFF4C, 0x80);
    EXPECT_EQ(bus.read(0xFF4C), 0x80);
    EXPECT_FALSE(mmu.dmgMode);

    bool callbackMode = true;
    bus.setCgbModeCallback([&callbackMode](bool mode) { callbackMode = mode; });

    bus.write(0xFF4C, 0x04);
    EXPECT_EQ(bus.read(0xFF4C), 0xFF);
    EXPECT_TRUE(mmu.dmgMode);
    EXPECT_FALSE(callbackMode);
}

TEST(GbCpuBusTest, Key0IsLockedAfterBootRomUnmap)
{
    FakeMmu mmu;
    qnemu::GbCpuBus bus(mmu, true);

    bus.write(0xFF50, 0x11);
    bus.write(0xFF4C, 0x04);

    EXPECT_EQ(bus.read(0xFF4C), 0x80);
    EXPECT_FALSE(mmu.dmgMode);
}

}  // namespace qnemuTest
