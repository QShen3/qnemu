/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>

#include <gtest/gtest.h>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/display/SgbFrameBuffer.h"
#include "qnemu/gb/sgb/GbSgb.h"

namespace qnemuTest
{

namespace
{

void sendBit(qnemu::GbSgb& sgb, bool one)
{
    sgb.write(one ? 0x10 : 0x20);
    sgb.write(0x30);
}

void sendPacket(qnemu::GbSgb& sgb, const std::array<uint8_t, 16>& packet)
{
    sgb.write(0x00);
    sgb.write(0x30);  // arm the receiver (old = 0 -> ready_for_write)
    for (uint8_t byte : packet) {
        for (int bit = 0; bit < 8; ++bit) {
            sendBit(sgb, (byte & (1U << bit)) != 0);
        }
    }
    // Commit the packet's stop bit.  The receiver dispatches when the next
    // bit commit follows a complete 16-byte packet.
    sendBit(sgb, false);
}

std::array<uint8_t, 16> paletteCommand()
{
    std::array<uint8_t, 16> packet {};
    packet[0] = 0x01;  // PAL01, one packet
    packet[1] = 0x1F;  // shared color 0: red RGB555
    packet[2] = 0x00;
    packet[3] = 0xFF;  // palette 0, color 1: white RGB555
    packet[4] = 0x7F;
    packet[9] = 0xE0;  // palette 1, color 1: green RGB555
    packet[10] = 0x03;
    return packet;
}

}  // namespace

TEST(GbSgbTest, Pal01UpdatesColor)
{
    qnemu::GbSgb sgb;
    sgb.reset();
    sendPacket(sgb, paletteCommand());

    EXPECT_EQ(sgb.colorForPixel(0, 0, 0), qnemu::rgb(255, 0, 0));
    EXPECT_EQ(sgb.colorForPixel(0, 0, 1), qnemu::rgb(255, 255, 255));
    EXPECT_EQ(sgb.colorForPixel(0, 0, 0), sgb.colorForPixel(0, 8, 0));
}

TEST(GbSgbTest, AttributeLineSelectsPalette)
{
    qnemu::GbSgb sgb;
    sgb.reset();
    sendPacket(sgb, paletteCommand());

    std::array<uint8_t, 16> attributes {};
    attributes[0] = 0x29;  // ATTR_LIN, one packet
    attributes[1] = 1;     // one entry
    attributes[2] = 0xA0;  // horizontal, palette 1, line 0
    sendPacket(sgb, attributes);

    EXPECT_EQ(sgb.colorForPixel(0, 0, 1), qnemu::rgb(0, 255, 0));
    EXPECT_EQ(sgb.colorForPixel(0, 8, 1), qnemu::rgb(255, 255, 255));
    EXPECT_EQ(sgb.colorForPixel(0, 0, 0), qnemu::rgb(255, 0, 0));
}

TEST(GbSgbTest, ChrAndPctTransfersLoadCustomBorder)
{
    qnemu::GbSgb sgb;
    sgb.reset();

    // CHR_TRN: one 4 KiB half of 4bpp tile data.  Tile 0 pixel (0, 0) uses
    // color 1.
    std::array<uint8_t, 0x2000> vram {};
    vram[0] = 0x80;
    std::array<uint8_t, 16> chr {};
    chr[0] = 0x99;  // CHR_TRN, one packet, low half
    sendPacket(sgb, chr);
    sgb.processTransfers(vram.data(), vram.size());
    sgb.processTransfers(vram.data(), vram.size());
    sgb.processTransfers(vram.data(), vram.size());

    // PCT_TRN: map every tile to tile 0, and palette color 1 to red.
    vram.fill(0);
    for (std::size_t tile = 0; tile < 32 * 28; ++tile) {
        vram[tile * 2] = 0;
        vram[tile * 2 + 1] = 0;
    }
    vram[0x802] = 0x1F;  // palette 0, color 1: red RGB555
    vram[0x803] = 0x00;
    std::array<uint8_t, 16> pct {};
    pct[0] = 0xA1;  // PCT_TRN
    sendPacket(sgb, pct);
    sgb.processTransfers(vram.data(), vram.size());
    sgb.processTransfers(vram.data(), vram.size());
    sgb.processTransfers(vram.data(), vram.size());

    qnemu::FrameBuffer game;
    game.fill(qnemu::rgb(0, 0, 0));
    qnemu::SgbFrameBuffer output;
    sgb.renderBorder(game, output);
    EXPECT_EQ(output.at(0, 0), qnemu::rgb(255, 0, 0));
}

TEST(GbSgbTest, BorderCompositionPlacesGameInCenter)
{
    qnemu::GbSgb sgb;
    sgb.reset();

    qnemu::FrameBuffer game;
    game.fill(qnemu::rgb(12, 34, 56));
    qnemu::SgbFrameBuffer output;
    sgb.renderBorder(game, output);

    EXPECT_EQ(output.at(48, 40), qnemu::rgb(12, 34, 56));
    EXPECT_EQ(output.at(48 + 159, 40 + 143), qnemu::rgb(12, 34, 56));
}

TEST(GbSgbTest, MaskEnableCommand)
{
    qnemu::GbSgb sgb;
    sgb.reset();

    std::array<uint8_t, 16> mask {};
    mask[0] = 0xB9;  // MASK_EN, one packet
    mask[1] = 2;     // black
    sendPacket(sgb, mask);

    EXPECT_EQ(sgb.maskMode(), 2);
}

}  // namespace qnemuTest
