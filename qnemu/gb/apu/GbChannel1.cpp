/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel1.h"

namespace qnemu
{

uint8_t GbChannel1::read(uint16_t address) const
{
    return 0;
}

void GbChannel1::write(uint16_t address, const uint8_t& value)
{

}

void GbChannel1::step()
{

}

void GbChannel1::reset()
{

}

uint8_t GbChannel1::getData() const
{
    return data;
}

}  // namespace qnemu