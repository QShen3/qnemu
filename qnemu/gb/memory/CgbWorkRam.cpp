/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/CgbWorkRam.h"

namespace qnemu
{

CgbWorkRam::CgbWorkRam()
{
    CgbWorkRam::reset();
}

uint8_t CgbWorkRam::read(uint16_t address) const
{
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        return data_.at(0).at(address - WorkRamBank00Start);
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        return data_.at(selectedBank()).at(address - WorkRamBank01Start);
    }
    if (address >= EchoRamStart && address < 0xF000) {
        return data_.at(0).at(address - EchoRamStart);
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        return data_.at(selectedBank()).at(address - 0xF000);
    }
    if (address == 0xFF70) {
        return dmgMode_ ? 0xFF : static_cast<uint8_t>(0xF8 | workRamBank_);
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void CgbWorkRam::write(uint16_t address, const uint8_t& value)
{
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        data_.at(0).at(address - WorkRamBank00Start) = value;
        return;
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        data_.at(selectedBank()).at(address - WorkRamBank01Start) = value;
        return;
    }
    if (address >= EchoRamStart && address < 0xF000) {
        data_.at(0).at(address - EchoRamStart) = value;
        return;
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        data_.at(selectedBank()).at(address - 0xF000) = value;
        return;
    }
    if (address == 0xFF70) {
        if (!dmgMode_) {
            workRamBank_ = static_cast<uint8_t>(value & 0x07);
        }
        return;
    }

    assert(false && "Wrong address");
}

void CgbWorkRam::reset()
{
    // SVBK resets to $F8 (bank 0, which selects bank 1 for D000-DFFF).
    workRamBank_ = 0;
    dmgMode_ = false;
}

uint8_t CgbWorkRam::selectedBank() const
{
    if (dmgMode_) {
        return 1;
    }
    return workRamBank_ == 0 ? 1 : workRamBank_;
}

}  // namespace qnemu
