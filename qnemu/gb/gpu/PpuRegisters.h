/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

namespace LcdcBits
{
inline constexpr uint8_t bgWindowEnable = 0x01;
inline constexpr uint8_t objEnable = 0x02;
inline constexpr uint8_t objSize16 = 0x04;
inline constexpr uint8_t bgMapHigh = 0x08;
inline constexpr uint8_t tileDataHigh = 0x10;
inline constexpr uint8_t windowEnable = 0x20;
inline constexpr uint8_t windowMapHigh = 0x40;
inline constexpr uint8_t lcdEnable = 0x80;
}  // namespace LcdcBits

namespace StatBits
{
inline constexpr uint8_t modeMask = 0x03;
inline constexpr uint8_t lycCoincidence = 0x04;
inline constexpr uint8_t mode0Interrupt = 0x08;
inline constexpr uint8_t mode1Interrupt = 0x10;
inline constexpr uint8_t mode2Interrupt = 0x20;
inline constexpr uint8_t lycInterrupt = 0x40;
}  // namespace StatBits

/**
 * PPU-visible register file.  Plain bytes plus small accessors keep the bit
 * layout explicit and make the PPU easy to snapshot from tests.
 */
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct PpuRegisters
{
    uint8_t lcdc = 0x91;
    uint8_t stat = 0x85;
    uint8_t scy = 0x00;
    uint8_t scx = 0x00;
    uint8_t ly = 0x00;
    uint8_t lyc = 0x00;
    uint8_t wy = 0x00;
    uint8_t wx = 0x00;
    uint8_t bgp = 0xFC;
    uint8_t obp0 = 0xFF;
    uint8_t obp1 = 0xFF;
    uint8_t opri = 0x00;  // FF6C, CGB object priority mode

    bool lcdEnabled() const { return (lcdc & LcdcBits::lcdEnable) != 0; }
    bool bgWindowEnabled() const { return (lcdc & LcdcBits::bgWindowEnable) != 0; }
    bool objEnabled() const { return (lcdc & LcdcBits::objEnable) != 0; }
    bool objSize16() const { return (lcdc & LcdcBits::objSize16) != 0; }
    bool bgMapHigh() const { return (lcdc & LcdcBits::bgMapHigh) != 0; }
    bool tileDataHigh() const { return (lcdc & LcdcBits::tileDataHigh) != 0; }
    bool windowEnabled() const { return (lcdc & LcdcBits::windowEnable) != 0; }
    bool windowMapHigh() const { return (lcdc & LcdcBits::windowMapHigh) != 0; }

    uint8_t mode() const { return static_cast<uint8_t>(stat & StatBits::modeMask); }
    void setMode(uint8_t value)
    {
        stat = static_cast<uint8_t>((stat & static_cast<uint8_t>(~StatBits::modeMask))
            | (value & StatBits::modeMask));
    }

    bool lycCoincidence() const { return (stat & StatBits::lycCoincidence) != 0; }
    void setLycCoincidence(bool value)
    {
        if (value) {
            stat = static_cast<uint8_t>(stat | StatBits::lycCoincidence);
        } else {
            stat = static_cast<uint8_t>(stat & static_cast<uint8_t>(~StatBits::lycCoincidence));
        }
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace qnemu
