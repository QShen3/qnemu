/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/gpu/CgbCompatibilityPalette.h"
#include "qnemu/gb/gpu/PpuMemory.h"

namespace qnemuTest
{

namespace
{

std::array<uint8_t, 16> titleBytes(const char* text)
{
    std::array<uint8_t, 16> title {};
    for (std::size_t i = 0; i < title.size() && text[i] != '\0'; ++i) {
        title.at(i) = static_cast<uint8_t>(text[i]);
    }
    return title;
}

}  // namespace

TEST(CgbCompatibilityPaletteTest, TetrisUsesItsBootRomPalette)
{
    qnemu::CgbCompatibilityHeader header;
    header.title = titleBytes("TETRIS");
    header.oldLicensee = 0x01;

    const qnemu::DmgCompatibilityPalette palette = qnemu::cgbCompatibilityPalette(header);
    const std::array<uint16_t, 4> expected {0x7FFF, 0x03FF, 0x001F, 0x0000};

    EXPECT_EQ(palette.background, expected);
    EXPECT_EQ(palette.object0, expected);
    EXPECT_EQ(palette.object1, expected);
}

TEST(CgbCompatibilityPaletteTest, NonNintendoGamesUseTheDefaultCombination)
{
    qnemu::CgbCompatibilityHeader header;
    header.title = titleBytes("HOMEBREW");
    header.oldLicensee = 0x00;

    const qnemu::DmgCompatibilityPalette palette = qnemu::cgbCompatibilityPalette(header);

    EXPECT_EQ(palette.background, (std::array<uint16_t, 4> {0x7FFF, 0x1BEF, 0x6180, 0x0000}));
    EXPECT_EQ(palette.object0, (std::array<uint16_t, 4> {0x7FFF, 0x421F, 0x1CF2, 0x0000}));
    EXPECT_EQ(palette.object1, palette.object0);
}

TEST(CgbCompatibilityPaletteTest, DuplicateChecksumsUseTheFourthTitleLetter)
{
    qnemu::CgbCompatibilityHeader header;
    header.title = titleBytes("POKEMON BLUE");
    header.oldLicensee = 0x01;

    const qnemu::DmgCompatibilityPalette palette = qnemu::cgbCompatibilityPalette(header);

    EXPECT_EQ(palette.object0, (std::array<uint16_t, 4> {0x7FFF, 0x421F, 0x1CF2, 0x0000}));
    EXPECT_EQ(palette.object1, (std::array<uint16_t, 4> {0x7FFF, 0x7E8C, 0x7C00, 0x0000}));
    EXPECT_EQ(palette.background, palette.object1);
}

TEST(PpuMemoryTest, DmgCompatibilityPaletteColorLookup)
{
    qnemu::PpuMemory memory;
    qnemu::DmgCompatibilityPalette palette;
    palette.background = {0x001F, 0x03E0, 0x7C00, 0x7FFF};
    palette.object0 = {0x0000, 0x001F, 0x03E0, 0x7C00};
    palette.object1 = {0x0000, 0x7C00, 0x03E0, 0x001F};
    memory.setDmgCompatibilityPalette(palette);

    EXPECT_TRUE(memory.dmgCompatibility());
    EXPECT_EQ(memory.dmgCompatibilityColor(false, 0, 0xE4, 0), qnemu::rgb(255, 0, 0));
    EXPECT_EQ(memory.dmgCompatibilityColor(false, 0, 0xE4, 1), qnemu::rgb(0, 255, 0));
    EXPECT_EQ(memory.dmgCompatibilityColor(false, 0, 0xE4, 2), qnemu::rgb(0, 0, 255));
    EXPECT_EQ(memory.dmgCompatibilityColor(false, 0, 0xE4, 3), qnemu::rgb(255, 255, 255));
    EXPECT_EQ(memory.dmgCompatibilityColor(true, 1, 0xE4, 2), qnemu::rgb(0, 255, 0));

    memory.clearDmgCompatibilityPalette();
    EXPECT_FALSE(memory.dmgCompatibility());
    EXPECT_EQ(memory.dmgColor(0xE4, 1), qnemu::rgb(170, 170, 170));
}

}  // namespace qnemuTest
