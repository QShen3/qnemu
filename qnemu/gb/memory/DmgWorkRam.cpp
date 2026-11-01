/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/DmgWorkRam.h"

namespace qnemu
{

DmgWorkRam::DmgWorkRam()
{
    DmgWorkRam::reset();
}

uint8_t DmgWorkRam::read(uint16_t address) const
{
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        return data_.at(0).at(address - WorkRamBank00Start);
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        return data_.at(1).at(address - WorkRamBank01Start);
    }
    if (address >= EchoRamStart && address < 0xF000) {
        return data_.at(0).at(address - EchoRamStart);
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        return data_.at(1).at(address - 0xF000);
    }
    if (address == 0xFF70) {
        return 0xFF;
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void DmgWorkRam::write(uint16_t address, const uint8_t& value)
{
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        data_.at(0).at(address - WorkRamBank00Start) = value;
        return;
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        data_.at(1).at(address - WorkRamBank01Start) = value;
        return;
    }
    if (address >= EchoRamStart && address < 0xF000) {
        data_.at(0).at(address - EchoRamStart) = value;
        return;
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        data_.at(1).at(address - 0xF000) = value;
        return;
    }
    if (address == 0xFF70) {
        return;
    }

    assert(false && "Wrong address");
}

void DmgWorkRam::step()
{
}

void DmgWorkRam::reset()
{
    // Work RAM contents are not cleared by a hardware reset.
}

}  // namespace qnemu
