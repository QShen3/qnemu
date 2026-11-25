/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/cartridge/mbc/GbAccelerometerState.h"

#include <algorithm>

namespace qnemu
{

void AccelerometerState::set(float x, float y)
{
    const std::lock_guard lock(mutex);
    xValue = std::clamp(x, -1.0F, 1.0F);
    yValue = std::clamp(y, -1.0F, 1.0F);
}

float AccelerometerState::x() const
{
    const std::lock_guard lock(mutex);
    return xValue;
}

float AccelerometerState::y() const
{
    const std::lock_guard lock(mutex);
    return yValue;
}

}  // namespace qnemu
