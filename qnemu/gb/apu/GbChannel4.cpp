/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel4.h"

namespace qnemu
{

uint8_t GbChannel4::read(uint16_t address) const
{
    return 0;
}

void GbChannel4::write(uint16_t address, const uint8_t& value)
{

}

void GbChannel4::step()
{

}

void GbChannel4::reset()
{

}

uint8_t GbChannel4::getData() const
{
    return data;
}

}  // namespace qnemu