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

    int steps = 0;
    bool doubleSpeed = false;
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

}  // namespace qnemuTest
