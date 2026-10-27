/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace qnemu
{

/**
 * Audio output sink.  The APU produces signed 16-bit stereo frames
 * (left, right, left, right, ...).  Implementations may buffer or block; the
 * APU calls addSamples() in batches of a few hundred frames.
 */
class SoundInterface
{
public:
    virtual ~SoundInterface() = default;

    virtual void enable() = 0;
    virtual void disable() = 0;

    /// Drop any buffered samples (used when a new ROM is loaded).
    virtual void flush() {}

    virtual void addSamples(const int16_t* samples, std::size_t frameCount) = 0;
};

}  // namespace qnemu
