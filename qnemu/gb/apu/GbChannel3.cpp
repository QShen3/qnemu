/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel3.h"

namespace qnemu
{

uint8_t GbChannel3::read(uint16_t address) const
{
    return 0;
}

void GbChannel3::write(uint16_t address, const uint8_t& value)
{

}

void GbChannel3::step()
{

}

void GbChannel3::reset()
{

}

uint8_t GbChannel3::getData() const
{
    return data;
}

}  // namespace qnemu