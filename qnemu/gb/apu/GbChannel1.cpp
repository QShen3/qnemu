/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel1.h"

namespace qnemu
{

GbChannel1::GbChannel1() : data(0)
{
}

uint8_t GbChannel1::read(uint16_t address) const
{
    if (address == 0xFF10) {
        return registers.channel1Sweep;
    } else if (address == 0xFF11) {
        return registers.channel1LengthTimerAndDutyCycle;
    } else if (address == 0xFF12) {
        return registers.channel1VolumeAndEnvelope;
    } else if (address == 0xFF13) {
        return registers.channel1PeriodLow;
    } else if (address == 0xFF14) {
        return registers.channel1PeriodHighAndControl;
    }
    return 0xFF;
}

void GbChannel1::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF10) {
        registers.channel1Sweep = value;
    } else if (address == 0xFF11) {
        registers.channel1LengthTimerAndDutyCycle = value;
    } else if (address == 0xFF12) {
        registers.channel1VolumeAndEnvelope = value;
    } else if (address == 0xFF13) {
        registers.channel1PeriodLow = value;
    } else if (address == 0xFF14) {
        registers.channel1PeriodHighAndControl = value;
    }
}

void GbChannel1::step()
{

}

void GbChannel1::reset()
{
    registers.channel1Sweep = 0x80;
    registers.channel1LengthTimerAndDutyCycle = 0xBF;
    registers.channel1VolumeAndEnvelope = 0xF3;
    registers.channel1PeriodLow = 0xFF;
    registers.channel1PeriodHighAndControl = 0xBF;
}

uint8_t GbChannel1::getData() const
{
    return data;
}

}  // namespace qnemu