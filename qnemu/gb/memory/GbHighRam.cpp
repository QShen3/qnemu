/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/const.h"
#include "qnemu/gb/memory/GbHighRam.h"

namespace qnemu
{

uint8_t GbHighRam::read(uint16_t address) const
{
    assert(address >= HighRamStart && address <= HighRamEnd && "Wrong address");
    return data.at(address - HighRamStart);
}

void GbHighRam::write(uint16_t address, const uint8_t& value)
{
    assert(address >= HighRamStart && address <= HighRamEnd && "Wrong address");
    data.at(address - HighRamStart) = value;
}

void GbHighRam::step()
{
}

void GbHighRam::reset()
{
}

}  // namespace qnemu
