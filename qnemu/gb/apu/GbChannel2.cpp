/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel2.h"

namespace qnemu
{

uint8_t GbChannel2::read(uint16_t address) const
{
    if (address == 0xFF16) {
        return registers.channel2LengthTimerAndDutyCycle;
    } else if (address == 0xFF17) {
        return registers.channel2VolumeAndEnvelope;
    } else if (address == 0xFF18) {
        return registers.channel2PeriodLow;
    } else if (address == 0xFF19) {
        return registers.channel2PeriodHighAndControl;
    }
    return 0xFF;
}

void GbChannel2::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF16) {
        registers.channel2LengthTimerAndDutyCycle = value;
    } else if (address == 0xFF17) {
        registers.channel2VolumeAndEnvelope = value;
    } else if (address == 0xFF18) {
        registers.channel2PeriodLow = value;
    } else if (address == 0xFF19) {
        registers.channel2PeriodHighAndControl = value;
    }
}

void GbChannel2::step()
{

}

void GbChannel2::reset()
{
    registers.channel2LengthTimerAndDutyCycle = 0x3F;
    registers.channel2VolumeAndEnvelope = 0;
    registers.channel2PeriodLow = 0xFF;
    registers.channel2PeriodHighAndControl = 0xBF;
}

uint8_t GbChannel2::getData() const
{
    return data;
}

}  // namespace qnemu