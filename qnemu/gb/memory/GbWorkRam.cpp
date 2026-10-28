/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemu
{

GbWorkRam::GbWorkRam()
{
    GbWorkRam::reset();
}

uint8_t GbWorkRam::read(uint16_t address) const
{
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        return data.at(0).at(address - WorkRamBank00Start);
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        return data.at(selectedBank()).at(address - WorkRamBank01Start);
    }
    if (address >= EchoRamStart && address < 0xF000) {
        return data.at(0).at(address - EchoRamStart);
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        return data.at(selectedBank()).at(address - 0xF000);
    }
    if (address == 0xFF70) {
        return cgb ? static_cast<uint8_t>(0xF8 | workRamBank) : 0xFF;
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbWorkRam::write(uint16_t address, const uint8_t& value)
{
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        data.at(0).at(address - WorkRamBank00Start) = value;
        return;
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        data.at(selectedBank()).at(address - WorkRamBank01Start) = value;
        return;
    }
    if (address >= EchoRamStart && address < 0xF000) {
        data.at(0).at(address - EchoRamStart) = value;
        return;
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        data.at(selectedBank()).at(address - 0xF000) = value;
        return;
    }
    if (address == 0xFF70) {
        if (cgb) {
            workRamBank = static_cast<uint8_t>(value & 0x07);
        }
        return;
    }

    assert(false && "Wrong address");
}

void GbWorkRam::step()
{
}

void GbWorkRam::reset()
{
    // SVBK resets to $F8 (bank 0, which selects bank 1 for D000-DFFF).
    workRamBank = 0;
}

uint8_t GbWorkRam::selectedBank() const
{
    if (!cgb) {
        return 1;
    }
    return workRamBank == 0 ? 1 : workRamBank;
}

}  // namespace qnemu
