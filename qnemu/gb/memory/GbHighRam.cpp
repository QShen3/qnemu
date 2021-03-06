/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/memory/GbHighRam.h"

namespace qnemu
{

bool GbHighRam::accepts(uint16_t address) const
{
    return address >= 0xFF80 && address < 0xFFFF;
}

uint8_t GbHighRam::read(uint16_t address) const
{
    return data.at(address - 0xFF80);
}

void GbHighRam::write(uint16_t address, const uint8_t& value)
{
    data.at(address - 0xFF80) = value;
}

void GbHighRam::step()
{
}

void GbHighRam::reset()
{
}

}  // namespace qnemu
