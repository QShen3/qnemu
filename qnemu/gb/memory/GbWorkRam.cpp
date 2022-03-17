/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemu
{

GbWorkRam::GbWorkRam()
{
    GbWorkRam::reset();
}

uint8_t GbWorkRam::read(uint16_t address) const
{
    if (address >= 0xC000 && address < 0xD000) {
        return data.at(0).at(address - 0xC000);
    }
    if (address >= 0xD000 && address < 0xE000) {
        if (registers.workRamBank == 0) {
            return data.at(1).at(address - 0xD000);
        }
        return data.at(registers.workRamBank).at(address - 0xD000);
    }
    if (address >= 0xE000 && address < 0xF000) {
        return data.at(0).at(address - 0xE000);
    }
    if (address >= 0xF000 && address < 0xFE00) {
        if (registers.workRamBank == 0) {
            return data.at(1).at(address - 0xF000);
        }
        return data.at(registers.workRamBank).at(address - 0xF000);
    }
    if (address == 0xFF70) {
        return registers.workRamBank;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbWorkRam::write(uint16_t address, const uint8_t& value)
{
    if (address >= 0xC000 && address < 0xD000) {
        data.at(0).at(address - 0xC000) = value;
    }
    if (address >= 0xD000 && address < 0xE000) {
        if (registers.workRamBank == 0) {
            data.at(1).at(address - 0xD000) = value;
        }
        data.at(registers.workRamBank).at(address - 0xD000) = value;
    }
    if (address >= 0xE000 && address < 0xF000) {
        data.at(0).at(address - 0xE000) = value;
    }
    if (address >= 0xF000 && address < 0xFE00) {
        if (registers.workRamBank == 0) {
            data.at(1).at(address - 0xF000) = value;
        }
        data.at(registers.workRamBank).at(address - 0xF000) = value;
    }
    if (address == 0xFF70) {
        registers.workRamBank = value;
    }
    assert(false && "Wrong address");
}

void GbWorkRam::reset()
{
    registers.workRamBank = 0x1;
}

void GbWorkRam::step()
{
}

bool GbWorkRam::accepts(uint16_t address) const
{
    if (address >= 0xC000 && address < 0xFE00) {
        return true;
    }
    if (address == 0xFF70) {
        return true;
    }
    return false;
}

}  // namespace qnemu
