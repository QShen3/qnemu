/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cmath>
#include <cstdint>
#include <numbers>
#include <iostream>

#include "qnemu/gb/apu/GbApu.h"

namespace qnemu
{

GbApu::GbApu(std::unique_ptr<SoundInterface> sound, std::array<std::unique_ptr<GbChannelInterface>, 4> channels) :
    sound(std::move(sound)),
    channels(std::move(channels))
{

}

GbApu::~GbApu()
{
}

uint8_t GbApu::read(uint16_t address) const
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
    } else if (address == 0xFF16) {
        return registers.channel2LengthTimerAndDutyCycle;
    } else if (address == 0xFF17) {
        return registers.channel2VolumeAndEnvelope;
    } else if (address == 0xFF18) {
        return registers.channel2PeriodLow;
    } else if (address == 0xFF19) {
        return registers.channel2PeriodHighAndControl;
    } else if (address == 0xFF1A) {
        return registers.channel3DacEnable;
    } else if (address == 0xFF1B) {
        return registers.channel3LengthTimer;
    } else if (address == 0xFF1C) {
        return registers.channel3OutputLevel;
    } else if (address == 0xFF1D) {
        return registers.channel3PeriodLow;
    } else if (address == 0xFF20) {
        return registers.channel4LengthTimer;
    } else if (address == 0xFF21) {
        return registers.channel4VolumeAndEnvelope;
    } else if (address == 0xFF22) {
        return registers.channel4FrequencyAndRandomness;
    } else if (address == 0xFF23) {
        return registers.channel4Control;
    } else if (address == 0xFF24) {
        return registers.masterVolumeAndVinPanning;
    } else if (address == 0xFF25) {
        return registers.soundPanning;
    } else if (address == 0xFF26) {
        return registers.audioMasterControl;
    }
    return 0xFF;
}

void GbApu::write(uint16_t address, const uint8_t& value)
{
    if (!registers.audioOn && address != 0xFF26) {
        return;
    }
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
    } else if (address == 0xFF16) {
        registers.channel2LengthTimerAndDutyCycle = value;
    } else if (address == 0xFF17) {
        registers.channel2VolumeAndEnvelope = value;
    } else if (address == 0xFF18) {
        registers.channel2PeriodLow = value;
    } else if (address == 0xFF19) {
        registers.channel2PeriodHighAndControl = value;
    } else if (address == 0xFF1A) {
        registers.channel3DacEnable = value;
    } else if (address == 0xFF1B) {
        registers.channel3LengthTimer = value;
    } else if (address == 0xFF1C) {
        registers.channel3OutputLevel = value;
    } else if (address == 0xFF1D) {
        registers.channel3PeriodLow = value;
    } else if (address == 0xFF1E) {
        registers.channel3PeriodHighAndControl = value;
    } else if (address == 0xFF20) {
        registers.channel4LengthTimer = value;
    } else if (address == 0xFF21) {
        registers.channel4VolumeAndEnvelope = value;
    } else if (address == 0xFF22) {
        registers.channel4FrequencyAndRandomness = value;
    } else if (address == 0xFF23) {
        registers.channel4Control = value;
    } else if (address == 0xFF24) {
        registers.masterVolumeAndVinPanning = value;
    } else if (address == 0xFF25) {
        registers.soundPanning = value;
    } else if (address == 0xFF26) {
        registers.audioOn = (value >> 7);
        if (!registers.audioOn) {
            reset();
        }
    }
}

void GbApu::step()
{

}

void GbApu::reset()
{
    registers.channel1Sweep = 0x80;
    registers.channel1LengthTimerAndDutyCycle = 0xBF;
    registers.channel1VolumeAndEnvelope = 0xF3;
    registers.channel1PeriodLow = 0xFF;
    registers.channel1PeriodHighAndControl = 0xBF;

    registers.channel2LengthTimerAndDutyCycle = 0x3F;
    registers.channel2VolumeAndEnvelope = 0;
    registers.channel2PeriodLow = 0xFF;
    registers.channel2PeriodHighAndControl = 0xBF;

    registers.channel3DacEnable = 0x7F;
    registers.channel3LengthTimer = 0xFF;
    registers.channel3OutputLevel = 0x9F;
    registers.channel3PeriodLow = 0xFF;
    registers.channel3PeriodHighAndControl = 0xBF;

    registers.channel4LengthTimer = 0xFF;
    registers.channel4VolumeAndEnvelope = 0;
    registers.channel4FrequencyAndRandomness = 0;
    registers.channel4Control = 0xBF;

    registers.masterVolumeAndVinPanning = 0x77;
    registers.soundPanning = 0xF3;
    registers.audioMasterControl = 0xF1;
}

}  // namespace qnemu
