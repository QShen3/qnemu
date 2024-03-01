/* 
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbLengthTimer.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbLengthTimer::GbLengthTimer()
{
    reset();
}

void GbLengthTimer::step()
{
    counter++;
    if (counter == (CpuFrequency / 256)) {
        counter = 0;
        if (enabled && length > 0) {
            length--;
        }
    }
}

void GbLengthTimer::setLength(uint8_t newLength)
{
    if (newLength == 0) {
        length = 64;
    } else {
        length = newLength;
    }
}

void GbLengthTimer::enable()
{
    enabled = true;
    if (length > 0 && counter < (CpuFrequency / 256) / 2) {
        length--;
    }
}

void GbLengthTimer::disable()
{
    enabled = false;
}

void GbLengthTimer::reset()
{
    length = 0;
    counter = 0;
    enabled = true;
}

void GbLengthTimer::trigger()
{
    if (enabled && length == 0) {
        if (counter < (CpuFrequency / 256) / 2) {
            length = 64 - 1;
        } else {
            length = 64;
        }
    }
    if (!enabled && length ==0) {
        length = 64;
    }
}

uint8_t GbLengthTimer::getLength() const
{
    return length;
}

bool GbLengthTimer::isEnabled() const
{
    return enabled;
}

} // namespace qnemu