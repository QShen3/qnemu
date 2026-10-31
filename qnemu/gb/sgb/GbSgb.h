/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "qnemu/gb/cartridge/GbCartridgeHeader.h"

namespace qnemu
{

class FrameBuffer;
class SgbFrameBuffer;

/**
 * Super Game Boy command receiver and colorization state.
 *
 * The SGB is a DMG inside an SNES: the SNES boot ROM talks to the GB through
 * short packets written to the joypad register.  This class implements the
 * receiver side of that protocol and the palette/attribute state used by the
 * PPU.  Border transfers and the SGB boot animation are separate concerns.
 */
class GbSgb
{
public:
    void reset();
    void setHeader(const GbCartridgeHeader& header);

    /// Called for every CPU write to FF00.  The previous written value is
    /// tracked internally, because the packet protocol is edge based.
    void write(uint8_t value);

    /// Color for a DMG color index at screen pixel (x, y).
    uint32_t colorForPixel(int x, int y, uint8_t colorIndex) const;

    /// Advances a pending CHR_TRN/PCT_TRN/PAL_TRN/ATTR_TRN transfer.  The
    /// caller supplies the current 8 KiB VRAM image at VBlank.
    void processTransfers(const uint8_t* vram, std::size_t size);
    bool transferPending() const { return transferCountdown_ != 0; }

    /// Composes the default or custom SGB border around a finished frame.
    void renderBorder(const FrameBuffer& gameFrame, SgbFrameBuffer& output) const;

    uint8_t attributeForTile(int tileX, int tileY) const;
    uint8_t maskMode() const { return maskMode_; }
    uint8_t playerCount() const { return playerCount_; }

private:
    static constexpr std::size_t PacketSize = 16;
    static constexpr std::size_t CommandSize = PacketSize * 7;
    static constexpr std::size_t AttributeTiles = 20 * 18;

    void commandReady();
    void palCommand(std::size_t first, std::size_t second);
    void loadAttributeFile(std::size_t fileIndex);

    std::array<uint8_t, CommandSize> command_ {};
    std::size_t writeIndex_ = 0;
    bool readyForWrite_ = false;
    bool readyForStop_ = false;
    bool disableCommands_ = false;
    uint8_t lastValue_ = 0;

    std::array<uint16_t, 4 * 4> effectivePalettes_ {};
    std::array<uint16_t, 4 * 512> ramPalettes_ {};
    std::array<uint8_t, AttributeTiles> attributeMap_ {};
    std::array<uint8_t, 0xFD2> attributeFiles_ {};
    std::array<uint16_t, 32 * 28> borderMap_ {};
    std::array<uint16_t, 16 * 4> borderPalette_ {};
    std::array<uint8_t, 0x2000> borderTiles_ {};

    uint8_t maskMode_ = 0;
    uint8_t playerCount_ = 1;
    uint8_t currentPlayer_ = 0;
    uint8_t transferCountdown_ = 0;
    uint8_t transferDestination_ = 0;
};

}  // namespace qnemu
