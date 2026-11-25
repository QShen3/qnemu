/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <mutex>

namespace qnemu
{

/** Thread-safe normalized accelerometer input shared with the cartridge. */
class AccelerometerState
{
public:
    /// X is positive to the left, Y is positive upwards, both in g units.
    void set(float x, float y);

    float x() const;
    float y() const;

private:
    mutable std::mutex mutex;
    float xValue = 0.0F;
    float yValue = 0.0F;
};

}  // namespace qnemu
