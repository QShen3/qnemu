/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * PPU-owned memory: both VRAM banks, OAM and the CGB palette RAM.
 * It deliberately knows nothing about cartridges, the MMU or Qt.
 */
class PpuMemory
{
public:
    PpuMemory();

    void reset();

    uint8_t readVram(uint16_t address) const;
    void writeVram(uint16_t address, uint8_t value);

    uint8_t readVramByte(uint8_t bank, uint16_t offset) const;
    void writeVramByte(uint8_t bank, uint16_t offset, uint8_t value);

    uint8_t readOam(uint16_t address) const;
    void writeOam(uint16_t address, uint8_t value);
    uint8_t readOamByte(uint8_t offset) const;
    void writeOamByte(uint8_t offset, uint8_t value);

    uint8_t vramBank() const { return vramBank_; }
    void setVramBank(uint8_t bank) { vramBank_ = static_cast<uint8_t>(bank & 0x01); }

    uint8_t readPaletteSpec(bool object) const;
    void writePaletteSpec(bool object, uint8_t value);
    uint8_t readPaletteData(bool object) const;
    void writePaletteData(bool object, uint8_t value);

    uint32_t dmgColor(uint8_t palette, uint8_t colorIndex) const;
    uint32_t cgbColor(bool object, uint8_t palette, uint8_t colorIndex) const;

private:
    std::array<std::array<uint8_t, VideoRamBankSize>, 2> videoRam_{};
    std::array<uint8_t, OamSize> oam_{};
    std::array<uint8_t, 0x40> backgroundPalette_{};
    std::array<uint8_t, 0x40> objectPalette_{};
    uint8_t vramBank_ = 0x00;
    uint8_t backgroundPaletteSpec_ = 0x00;
    uint8_t objectPaletteSpec_ = 0x00;
};

}  // namespace qnemu
