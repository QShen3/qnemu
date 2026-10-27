/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbVolumeEnvelope.h"

namespace qnemu
{

void GbVolumeEnvelope::reset()
{
    initialVolume = 0;
    volume = 0;
    pace = 0;
    timer = 0;
    direction = false;
    active = false;
    dacEnabled = false;
}

void GbVolumeEnvelope::configure(uint8_t registerValue)
{
    initialVolume = static_cast<uint8_t>(registerValue >> 4);
    direction = (registerValue & 0x08) != 0;
    pace = static_cast<uint8_t>(registerValue & 0x07);
    dacEnabled = (registerValue & 0xF8) != 0;

    // Writing the volume register does not change the current volume; that
    // only happens on trigger (or through the hardware's "zombie" mode, which
    // is intentionally not modelled).
}

void GbVolumeEnvelope::trigger()
{
    volume = initialVolume;
    timer = pace;
    active = pace != 0;
}

void GbVolumeEnvelope::clock()
{
    if (!active) {
        return;
    }

    if (timer > 0) {
        --timer;
    }
    if (timer > 0) {
        return;
    }

    if (direction) {
        if (volume < 15) {
            ++volume;
        } else {
            active = false;
            return;
        }
    } else {
        if (volume > 0) {
            --volume;
        } else {
            active = false;
            return;
        }
    }

    timer = pace;
}

}  // namespace qnemu
