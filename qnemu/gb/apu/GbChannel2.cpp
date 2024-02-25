/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel2.h"

namespace qnemu
{

uint8_t GbChannel2::read(uint16_t address) const
{
    return 0;
}

void GbChannel2::write(uint16_t address, const uint8_t& value)
{

}

void GbChannel2::step()
{

}

void GbChannel2::reset()
{

}

uint8_t GbChannel2::getData() const
{
    return data;
}

}  // namespace qnemu