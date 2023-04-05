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
        if (registers.workRamBank == 0) {
            return data.at(1).at(address - WorkRamBank01Start);
        }
        return data.at(registers.workRamBank).at(address - WorkRamBank01Start);
    }
    if (address >= EchoRamStart && address < 0xF000) {
        return data.at(0).at(address - EchoRamStart);
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
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
    if (address >= WorkRamBank00Start && address <= WorkRamBank00End) {
        data.at(0).at(address - WorkRamBank00Start) = value;
    }
    if (address >= WorkRamBank01Start && address <= WorkRamBank01End) {
        if (registers.workRamBank == 0) {
            data.at(1).at(address - WorkRamBank01Start) = value;
        }
        data.at(registers.workRamBank).at(address - WorkRamBank01Start) = value;
    }
    if (address >= EchoRamStart && address < 0xF000) {
        data.at(0).at(address - EchoRamStart) = value;
    }
    if (address >= 0xF000 && address <= EchoRamEnd) {
        if (registers.workRamBank == 0) {
            data.at(1).at(address - 0xF000) = value;
        }
        data.at(registers.workRamBank).at(address - 0xF000) = value;
    }
    if (address == 0xFF70) {
        registers.workRamBank = value & 0b111;
    }
}

void GbWorkRam::step()
{
}

void GbWorkRam::reset()
{
    registers.workRamBank = 0x1;
}

}  // namespace qnemu
