/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel4.h"

namespace qnemu
{

GbChannel4::GbChannel4() : data(0)
{
    registers.channel4LengthTimer = 0xFF;
    registers.channel4VolumeAndEnvelope = 0;
    registers.channel4FrequencyAndRandomness = 0;
    registers.channel4Control = 0xBF;
}

uint8_t GbChannel4::read(uint16_t address) const
{
    if (address == 0xFF20) {
        return registers.channel4LengthTimer;
    } else if (address == 0xFF21) {
        return registers.channel4VolumeAndEnvelope;
    } else if (address == 0xFF22) {
        return registers.channel4FrequencyAndRandomness;
    } else if (address == 0xFF23) {
        return registers.channel4Control;
    }
    return 0xFF;
}

void GbChannel4::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF20) {
        registers.channel4LengthTimer = value;
    } else if (address == 0xFF21) {
        registers.channel4VolumeAndEnvelope = value;
    } else if (address == 0xFF22) {
        registers.channel4FrequencyAndRandomness = value;
    } else if (address == 0xFF23) {
        registers.channel4Control = value;
    }
}

void GbChannel4::step()
{

}

void GbChannel4::reset()
{
    registers.channel4LengthTimer = 0xFF;
    registers.channel4VolumeAndEnvelope = 0;
    registers.channel4FrequencyAndRandomness = 0;
    registers.channel4Control = 0xBF;
}

uint8_t GbChannel4::getData() const
{
    return data;
}

}  // namespace qnemu