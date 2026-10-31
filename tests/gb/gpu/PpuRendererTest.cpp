/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/PpuMemory.h"
#include "qnemu/gb/gpu/PpuRegisters.h"
#include "qnemu/gb/gpu/PpuRenderer.h"
#include "qnemu/gb/sgb/GbSgb.h"

namespace qnemuTest
{

namespace
{
constexpr uint8_t dmgLcdc = qnemu::LcdcBits::lcdEnable
    | qnemu::LcdcBits::bgWindowEnable
    | qnemu::LcdcBits::tileDataHigh;

void setCgbColor(qnemu::PpuMemory& memory, bool object,
    uint8_t palette, uint8_t colorIndex, uint16_t color)
{
    const uint8_t index = static_cast<uint8_t>(palette * 8 + colorIndex * 2);
    memory.writePaletteSpec(object, static_cast<uint8_t>(0x80 | index));
    memory.writePaletteData(object, static_cast<uint8_t>(color & 0xFF));
    memory.writePaletteData(object, static_cast<uint8_t>(color >> 8));
}

void setSprite(qnemu::PpuMemory& memory, uint8_t oamIndex,
    uint8_t y, uint8_t x, uint8_t tile, uint8_t attributes)
{
    memory.writeOamByte(oamIndex, y);
    memory.writeOamByte(static_cast<uint8_t>(oamIndex + 1), x);
    memory.writeOamByte(static_cast<uint8_t>(oamIndex + 2), tile);
    memory.writeOamByte(static_cast<uint8_t>(oamIndex + 3), attributes);
}
}  // namespace

TEST(PpuRendererTest, DmgBackgroundUsesTileDataAndPalette)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(false);

    registers.lcdc = dmgLcdc;
    registers.bgp = 0xE4;
    memory.writeVramByte(0, 0x1800, 0x00);
    memory.writeVramByte(0, 0x0000, 0x80);
    memory.writeVramByte(0, 0x0001, 0x00);

    EXPECT_FALSE(renderer.renderLine(registers, memory, 0, 0, frame));
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(170, 170, 170));
    EXPECT_EQ(frame.at(1, 0), qnemu::rgb(255, 255, 255));
}

TEST(PpuRendererTest, SignedTileIndexUses8800Addressing)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(false);

    registers.lcdc = qnemu::LcdcBits::lcdEnable
        | qnemu::LcdcBits::bgWindowEnable;
    registers.bgp = 0xE4;
    memory.writeVramByte(0, 0x1800, 0xA1);
    const uint16_t tileOffset = static_cast<uint16_t>(
        0x1000 + static_cast<int8_t>(0xA1) * 16);
    memory.writeVramByte(0, tileOffset, 0x80);
    memory.writeVramByte(0, static_cast<uint16_t>(tileOffset + 1), 0x00);

    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(170, 170, 170));
}

TEST(PpuRendererTest, WindowOverridesBackgroundAndTracksVisibility)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(false);

    registers.lcdc = dmgLcdc
        | qnemu::LcdcBits::bgMapHigh
        | qnemu::LcdcBits::windowEnable;
    registers.bgp = 0xE4;
    registers.wx = 8;
    registers.wy = 0;

    // Background map at 0x9C00 points to a blank tile.
    memory.writeVramByte(0, 0x1C00, 0x00);
    // Window map at 0x9800 points to a tile whose first pixel is color 1.
    memory.writeVramByte(0, 0x1800, 0x01);
    memory.writeVramByte(0, 0x0010, 0x80);
    memory.writeVramByte(0, 0x0011, 0x00);

    EXPECT_TRUE(renderer.renderLine(registers, memory, 0, 0, frame));
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(255, 255, 255));
    EXPECT_EQ(frame.at(1, 0), qnemu::rgb(170, 170, 170));
}

TEST(PpuRendererTest, DmgObjectToBackgroundPriorityHidesSpriteOverColor)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(false);

    registers.lcdc = dmgLcdc | qnemu::LcdcBits::objEnable;
    registers.bgp = 0xE4;
    registers.obp0 = 0xE4;

    memory.writeVramByte(0, 0x1800, 0x00);
    memory.writeVramByte(0, 0x0000, 0x80);
    memory.writeVramByte(0, 0x0001, 0x00);
    memory.writeVramByte(0, 0x0010, 0xFF);
    memory.writeVramByte(0, 0x0011, 0xFF);
    setSprite(memory, 0, 16, 8, 1, 0x80);

    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(170, 170, 170));

    setSprite(memory, 0, 16, 8, 1, 0x00);
    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(0, 0, 0));
}

TEST(PpuRendererTest, SgbBackgroundUsesSgbPalette)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::GbSgb sgb;
    sgb.reset();
    qnemu::PpuRenderer renderer(false, &sgb);

    registers.lcdc = dmgLcdc;
    registers.bgp = 0xE4;
    memory.writeVramByte(0, 0x1800, 0x00);
    memory.writeVramByte(0, 0x0000, 0x80);
    memory.writeVramByte(0, 0x0001, 0x00);

    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), sgb.colorForPixel(0, 0, 1));
    EXPECT_NE(frame.at(0, 0), qnemu::rgb(170, 170, 170));
}

TEST(PpuRendererTest, CgbMasterAndBackgroundPriority)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(true);

    registers.lcdc = dmgLcdc | qnemu::LcdcBits::objEnable;
    setCgbColor(memory, false, 0, 1, 0x03E0);  // green
    setCgbColor(memory, true, 0, 3, 0x7C00);   // blue

    memory.writeVramByte(0, 0x1800, 0x00);
    memory.writeVramByte(1, 0x1800, 0x80);  // BG-to-OAM priority
    memory.writeVramByte(0, 0x0000, 0x80);
    memory.writeVramByte(0, 0x0001, 0x00);
    memory.writeVramByte(0, 0x0010, 0xFF);
    memory.writeVramByte(0, 0x0011, 0xFF);
    setSprite(memory, 0, 16, 8, 1, 0x00);

    // LCDC bit 0 is set: the background tile priority hides the object.
    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(0, 255, 0));

    // With the master priority disabled the BG attribute is ignored.
    registers.lcdc = static_cast<uint8_t>(registers.lcdc & ~qnemu::LcdcBits::bgWindowEnable);
    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(0, 0, 255));
}

TEST(PpuRendererTest, CgbObjectPriorityModeSelectsOamOrX)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(true);

    registers.lcdc = dmgLcdc | qnemu::LcdcBits::objEnable;
    setCgbColor(memory, true, 0, 1, 0x001F);  // red
    setCgbColor(memory, true, 0, 3, 0x7C00);  // blue
    memory.writeVramByte(0, 0x0010, 0x80);
    memory.writeVramByte(0, 0x0011, 0x00);
    memory.writeVramByte(0, 0x0020, 0xFF);
    memory.writeVramByte(0, 0x0021, 0xFF);

    // OAM entry 0 is at x=20, entry 1 at x=16; both cover x=12..15.
    setSprite(memory, 0, 16, 20, 1, 0x00);
    setSprite(memory, 4, 16, 16, 2, 0x00);

    registers.opri = 0;  // OAM order: entry 0 wins
    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(12, 0), qnemu::rgb(255, 0, 0));

    registers.opri = 1;  // X priority: entry 1 (x=16) wins
    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(12, 0), qnemu::rgb(0, 0, 255));
}

TEST(PpuRendererTest, EightBySixteenIgnoresTileIndexBitZero)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(false);

    registers.lcdc = dmgLcdc
        | qnemu::LcdcBits::objEnable
        | qnemu::LcdcBits::objSize16;
    registers.obp0 = 0xE4;
    memory.writeVramByte(0, 0x0020, 0x80);  // tile 2, color 1
    memory.writeVramByte(0, 0x0021, 0x00);
    memory.writeVramByte(0, 0x0030, 0xFF);  // tile 3, color 3
    memory.writeVramByte(0, 0x0031, 0xFF);
    setSprite(memory, 0, 16, 8, 3, 0x00);

    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(170, 170, 170));
}

TEST(PpuRendererTest, TenObjectLimitDropsTheEleventh)
{
    qnemu::PpuMemory memory;
    qnemu::PpuRegisters registers;
    qnemu::FrameBuffer frame;
    qnemu::PpuRenderer renderer(false);

    registers.lcdc = dmgLcdc | qnemu::LcdcBits::objEnable;
    registers.obp0 = 0xE4;
    memory.writeVramByte(0, 0x0010, 0x80);
    memory.writeVramByte(0, 0x0011, 0x00);
    for (uint8_t index = 0; index < 10; ++index) {
        setSprite(memory, static_cast<uint8_t>(index * 4), 16, 8, 0, 0x00);
    }
    setSprite(memory, 40, 16, 8, 1, 0x00);

    renderer.renderLine(registers, memory, 0, 0, frame);
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(255, 255, 255));
}

}  // namespace qnemuTest
