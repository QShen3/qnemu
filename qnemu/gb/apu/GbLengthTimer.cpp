/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbLengthTimer.h"

namespace qnemu
{

GbLengthTimer::GbLengthTimer(uint16_t maxLength) : maxLength(maxLength)
{
    reset();
}

void GbLengthTimer::reset()
{
    length = 0;
    enabled = false;
}

void GbLengthTimer::setLength(uint8_t registerValue)
{
    length = registerValue == 0
        ? maxLength
        : static_cast<uint16_t>(maxLength - registerValue);
}

void GbLengthTimer::setEnabled(bool value)
{
    enabled = value;
}

void GbLengthTimer::trigger()
{
    if (length == 0) {
        length = maxLength;
    }
}

void GbLengthTimer::clock()
{
    if (enabled && length > 0) {
        --length;
    }
}

}  // namespace qnemu
