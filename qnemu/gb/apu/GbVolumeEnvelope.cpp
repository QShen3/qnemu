/* 
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbVolumeEnvelope.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbVolumeEnvelope::GbVolumeEnvelope()
{
    reset();
}

void GbVolumeEnvelope::step()
{
    if (finished) {
        return;
    }

    if ((volume == 0 && direction == false) || (volume == 0b1111 && direction == true)) {
        finished = true;
        return;
    }

    counter++;
    if (counter == (CpuFrequency / 64 * pace)) {
        counter = 0;
        volume++;
    }
}

void GbVolumeEnvelope::setInitialVolume(uint8_t volume)
{
    initialVolume = volume;
}

void GbVolumeEnvelope::setDirection(bool newDirection)
{
    direction = newDirection;
}

void GbVolumeEnvelope::setPace(uint8_t newPace)
{
    pace = newPace;
}

void GbVolumeEnvelope::reset()
{
    initialVolume = 0;
    direction = false;
    pace = 0;

    finished = true;
    volume = 0;
    counter = 0;
}

void GbVolumeEnvelope::trigger()
{
    volume = initialVolume;
    counter = 0;
    finished = false;
}

uint8_t GbVolumeEnvelope::getVolume() const
{
    if (pace == 0) {
        return initialVolume;
    } else {
        return volume;
    }
}

} // namespace qnemu