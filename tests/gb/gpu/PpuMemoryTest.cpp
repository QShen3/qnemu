/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <gtest/gtest.h>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/PpuMemory.h"

namespace qnemuTest
{

TEST(PpuMemoryTest, DmgShadeOrder)
{
    qnemu::PpuMemory memory;

    EXPECT_EQ(memory.dmgColor(0xE4, 0), qnemu::rgb(255, 255, 255));
    EXPECT_EQ(memory.dmgColor(0xE4, 1), qnemu::rgb(170, 170, 170));
    EXPECT_EQ(memory.dmgColor(0xE4, 2), qnemu::rgb(85, 85, 85));
    EXPECT_EQ(memory.dmgColor(0xE4, 3), qnemu::rgb(0, 0, 0));
}

TEST(PpuMemoryTest, CgbPaletteAutoIncrementAndColor)
{
    qnemu::PpuMemory memory;
    memory.writePaletteSpec(false, 0x80);
    memory.writePaletteData(false, 0x1F);
    memory.writePaletteData(false, 0x00);

    EXPECT_EQ(memory.readPaletteSpec(false), 0x82);
    EXPECT_EQ(memory.cgbColor(false, 0, 0), qnemu::rgb(255, 0, 0));

    memory.writePaletteSpec(true, 0x86);
    memory.writePaletteData(true, 0x00);
    memory.writePaletteData(true, 0x7C);

    EXPECT_EQ(memory.cgbColor(true, 0, 3), qnemu::rgb(0, 0, 255));
}

TEST(PpuMemoryTest, CgbPaletteDataReadsTheIndexedByte)
{
    qnemu::PpuMemory memory;
    memory.writePaletteSpec(false, 0x03);
    memory.writePaletteData(false, 0x5A);
    EXPECT_EQ(memory.readPaletteData(false), 0x5A);
}

TEST(PpuMemoryTest, VramBanksAreIndependent)
{
    qnemu::PpuMemory memory;
    memory.setVramBank(1);
    memory.writeVram(0x8000, 0xAB);
    memory.setVramBank(0);

    EXPECT_EQ(memory.readVram(0x8000), 0x00);
    memory.setVramBank(1);
    EXPECT_EQ(memory.readVram(0x8000), 0xAB);
}

}  // namespace qnemuTest
