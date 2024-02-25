/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel3.h"

namespace qnemu
{

uint8_t GbChannel3::read(uint16_t address) const
{
    if (address == 0xFF1A) {
        return registers.channel3DacEnable;
    } else if (address == 0xFF1B) {
        return registers.channel3LengthTimer;
    } else if (address == 0xFF1C) {
        return registers.channel3OutputLevel;
    } else if (address == 0xFF1D) {
        return registers.channel3PeriodLow;
    } else if (address == 0xFF1E) {
        return registers.channel3PeriodHighAndControl;
    }
    return 0xFF;
}

void GbChannel3::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF1A) {
        registers.channel3DacEnable = value;
    } else if (address == 0xFF1B) {
        registers.channel3LengthTimer = value;
    } else if (address == 0xFF1C) {
        registers.channel3OutputLevel = value;
    } else if (address == 0xFF1D) {
        registers.channel3PeriodLow = value;
    } else if (address == 0xFF1E) {
        registers.channel3PeriodHighAndControl = value;
    }
}

void GbChannel3::step()
{

}

void GbChannel3::reset()
{
    registers.channel3DacEnable = 0x7F;
    registers.channel3LengthTimer = 0xFF;
    registers.channel3OutputLevel = 0x9F;
    registers.channel3PeriodLow = 0xFF;
    registers.channel3PeriodHighAndControl = 0xBF;
}

uint8_t GbChannel3::getData() const
{
    return data;
}

}  // namespace qnemu