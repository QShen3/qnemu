/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

/**
 * C000-DFFF Work RAM and its E000-FDFF echo, plus the CGB SVBK register.
 *
 * In CGB mode bank 0 is fixed at C000-CFFF and bank 1-7 is selected at
 * D000-DFFF; SVBK=0 maps bank 1, and SVBK bits 3-7 read as 1.  In DMG mode
 * D000-DFFF is the fixed upper 4 KiB and SVBK reads $FF.
 */
class GbWorkRam : public GbDeviceInterface
{
public:
    GbWorkRam();
    ~GbWorkRam() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    /// DMG compatibility mode ignores SVBK and does not bank D000-DFFF.
    void setCgb(bool value) { cgb = value; }

private:
    uint8_t selectedBank() const;

    bool cgb = true;
    uint8_t workRamBank = 0;
    std::array<std::array<uint8_t, 0x1000>, 8> data{};
};

}  // namespace qnemu
