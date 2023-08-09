/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/apu/GbApu.h"

namespace qnemu
{

uint8_t GbApu::read(uint16_t address) const
{
    return 0xFF;
}

void GbApu::write(uint16_t address, const uint8_t& value)
{

}

void GbApu::step()
{

}

void GbApu::reset()
{

}

}  // namespace qnemu
