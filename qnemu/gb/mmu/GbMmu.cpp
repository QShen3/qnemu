/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/mmu/GbMmu.h"

namespace qnemu
{

GbMmu::GbMmu()
{

}

uint8_t GbMmu::read(uint16_t address) const
{
    return 0;
}

void GbMmu::write(uint16_t address, const uint8_t& value)
{
    return;
}

}  // namespace qnemu