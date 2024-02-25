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

uint8_t GbApu::read(uint16_t address) const
{
    if (address >= 0xFF10 && address <= 0xFF14) {
        channels[0]->read(address);
    } else if (address >= 0xFF16 && address <= 0xFF19) {
        channels[1]->read(address);
    } else if (address >= 0xFF1A && address <= 0xFF1E) {
        channels[2]->read(address);
    } else if (address >= 0xFF20 && address <= 0xFF23) {
        channels[3]->read(address);
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
    if (address >= 0xFF10 && address <= 0xFF14) {
        channels[0]->write(address, value);
    } else if (address >= 0xFF16 && address <= 0xFF19) {
        channels[1]->write(address, value);
    } else if (address >= 0xFF1A && address <= 0xFF1E) {
        channels[2]->write(address, value);
    } else if (address >= 0xFF20 && address <= 0xFF23) {
        channels[3]->write(address, value);
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
    for (auto& channel : channels) {
        channel->step();
    }
}

void GbApu::reset()
{
    for (auto& channel : channels) {
        channel->reset();
    }

    registers.masterVolumeAndVinPanning = 0x77;
    registers.soundPanning = 0xF3;
    registers.audioMasterControl = 0xF1;
}

}  // namespace qnemu
