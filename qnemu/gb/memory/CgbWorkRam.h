/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/memory/DmgWorkRam.h"

namespace qnemu
{

/**
 * CGB work RAM.
 *
 * The CGB adds eight 4 KiB banks and SVBK (FF70).  Bank 0 is fixed at
 * C000-CFFF; D000-DFFF selects bank 1-7, with SVBK=0 mapping bank 1.  In DMG
 * compatibility mode SVBK follows DMG behavior and the upper bank is fixed.
 */
class CgbWorkRam final : public DmgWorkRam
{
public:
    CgbWorkRam();

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void reset() override;

    void setDmgCompatibility(bool enabled) { dmgMode_ = enabled; }
    bool dmgCompatibility() const { return dmgMode_; }

private:
    uint8_t selectedBank() const;

    bool dmgMode_ = false;
    uint8_t workRamBank_ = 0;
};

}  // namespace qnemu
