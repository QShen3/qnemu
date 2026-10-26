/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "tests/support/MemoryDisplay.h"

namespace qnemuTest
{

class GbPpuTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ppu = std::make_unique<qnemu::GbPpu>(false, display, interruptHandler);
    }

    std::shared_ptr<MemoryDisplay> display = std::make_shared<MemoryDisplay>();
    std::shared_ptr<qnemu::GbInterruptHandler> interruptHandler = std::make_shared<qnemu::GbInterruptHandler>();
    std::unique_ptr<qnemu::GbPpu> ppu;
};

TEST_F(GbPpuTest, ResetState)
{
    EXPECT_EQ(ppu->read(0xFF40), 0x91);
    EXPECT_EQ(ppu->read(0xFF44), 0x00);
    EXPECT_EQ(ppu->currentMode(), 2);
    EXPECT_TRUE(ppu->isLcdEnable());
    EXPECT_EQ(ppu->frameCount(), 0U);
}

TEST_F(GbPpuTest, ModeTransitionsFollowTheDotClock)
{
    for (int i = 0; i < 80; ++i) {
        ppu->step();
    }
    EXPECT_EQ(ppu->currentMode(), 2);

    ppu->step();
    EXPECT_EQ(ppu->currentMode(), 3);

    while (ppu->currentMode() != 0) {
        ppu->step();
    }
    EXPECT_EQ(ppu->read(0xFF44), 0x00);

    for (int i = 0; i < 204; ++i) {
        ppu->step();
    }
    EXPECT_EQ(ppu->read(0xFF44), 0x01);
}

TEST_F(GbPpuTest, VBlankPresentsOneFrame)
{
    for (int i = 0; i < 70224 * 2 && ppu->frameCount() == 0; ++i) {
        ppu->step();
    }

    EXPECT_EQ(ppu->frameCount(), 1U);
    EXPECT_EQ(display->frameCount, 1U);
    EXPECT_EQ(ppu->read(0xFF44), 144);
    EXPECT_EQ(ppu->currentMode(), 1);
}

TEST_F(GbPpuTest, OamDmaCopiesOneBytePerFourCycles)
{
    ppu->write(0xFF40, 0x00);
    ppu->setExternalRead([](uint16_t address) {
        return static_cast<uint8_t>(address & 0xFF);
    });
    ppu->write(0xFF46, 0x02);

    for (int i = 0; i < 640; ++i) {
        ppu->step();
    }

    EXPECT_EQ(ppu->read(0xFF46), 0x02);
    EXPECT_EQ(ppu->read(0xFE00), 0x00);
    EXPECT_EQ(ppu->read(0xFE01), 0x01);
    EXPECT_EQ(ppu->read(0xFE9F), 0x9F);
}

TEST_F(GbPpuTest, HdmaCopiesAWholeBlock)
{
    qnemu::GbPpu cgbPpu(true, display, interruptHandler);
    cgbPpu.write(0xFF40, 0x00);
    cgbPpu.setExternalRead([](uint16_t address) {
        return static_cast<uint8_t>(address & 0xFF);
    });
    cgbPpu.write(0xFF51, 0x02);
    cgbPpu.write(0xFF52, 0x00);
    cgbPpu.write(0xFF53, 0x00);
    cgbPpu.write(0xFF54, 0x00);
    cgbPpu.write(0xFF55, 0x00);

    for (int i = 0; i < 32; ++i) {
        cgbPpu.step();
    }

    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(cgbPpu.read(static_cast<uint16_t>(0x8000 + i)), i);
    }
    EXPECT_EQ(cgbPpu.read(0xFF55), 0x80);
}

TEST_F(GbPpuTest, LycInterruptIsRequestedOnCoincidence)
{
    interruptHandler->write(0xFFFF, 0x02);
    ppu->write(0xFF45, 0x01);
    ppu->write(0xFF41, 0x40);

    for (int i = 0; i < 456 * 2; ++i) {
        ppu->step();
    }

    EXPECT_NE(interruptHandler->read(0xFF0F) & 0x02, 0);
}

TEST_F(GbPpuTest, DisablingLcdResetsLyAndMode)
{
    ppu->write(0xFF40, 0x00);
    ppu->step();

    EXPECT_EQ(ppu->read(0xFF44), 0x00);
    EXPECT_EQ(ppu->currentMode(), 0);
    EXPECT_FALSE(ppu->isLcdEnable());
}

}  // namespace qnemuTest
