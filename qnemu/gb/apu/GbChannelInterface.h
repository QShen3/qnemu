/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

/**
 * Common interface for the four APU channels.
 *
 * `step()` is called once per T-cycle.  The APU's frame sequencer calls the
 * clock*() hooks at the rates defined by the hardware:
 *   - clockLength():    256 Hz
 *   - clockSweep():     128 Hz (channel 1 only)
 *   - clockEnvelope():   64 Hz (channels 1, 2 and 4)
 *
 * `getData()` is the channel's current 4-bit digital output (0..15).  The APU
 * converts it to an analog level in the mixer.
 */
class GbChannelInterface : public GbDeviceInterface
{
public:
    ~GbChannelInterface() override = default;

    virtual uint8_t getData() const = 0;
    virtual bool isEnabled() const = 0;
    virtual bool isDacEnabled() const = 0;

    virtual void clockLength() {}
    virtual void clockEnvelope() {}
    virtual void clockSweep() {}

    /**
     * Power the APU down.  Most channels simply reset; the wave channel keeps
     * its wave RAM because the hardware preserves it while NR52 bit 7 is 0.
     */
    virtual void powerOff() { reset(); }
};

}  // namespace qnemu
