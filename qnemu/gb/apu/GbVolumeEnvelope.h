/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

/**
 * Volume envelope shared by CH1, CH2 and CH4 (CH3 has a fixed volume instead).
 *
 * The envelope is clocked at 64 Hz by the frame sequencer.  With a pace of N
 * the volume changes once every N clocks.
 */
class GbVolumeEnvelope
{
public:
    GbVolumeEnvelope() = default;
    ~GbVolumeEnvelope() = default;

    void reset();
    void configure(uint8_t registerValue);
    void trigger();
    void clock();

    uint8_t getVolume() const { return volume; }
    bool isDacEnabled() const { return dacEnabled; }

private:
    uint8_t initialVolume = 0;
    uint8_t volume = 0;
    uint8_t pace = 0;
    uint8_t timer = 0;
    bool direction = false;
    bool active = false;
    bool dacEnabled = false;
};

}  // namespace qnemu
