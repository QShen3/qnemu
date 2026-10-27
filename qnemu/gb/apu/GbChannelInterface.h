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
     * Tell the channel whether the frame sequencer's next step clocks the
     * length timer.  NRx4 writes use this for the "extra length clocking"
     * quirk, and triggers use it when they reload a frozen length counter.
     */
    virtual void setLengthClockPhase(bool /*nextStepClocksLength*/) {}

    /// The wave channel needs to know whether it is running on DMG or CGB:
    /// DMG only allows wave RAM access when it coincides with an internal read.
    virtual void setCgb(bool /*cgb*/) {}

    /// DMG only: NRx1 length writes still work while the APU is powered off.
    /// Only the length counter is affected; readable register bits stay clear.
    virtual void writeLengthWhileOff(uint8_t /*value*/) {}

    /**
     * Power the APU down.  On DMG the length counters survive; on CGB they
     * are cleared.  The wave channel always keeps its wave RAM.
     */
    virtual void powerOff(bool /*preserveLength*/ = false) { reset(); }
};

}  // namespace qnemu
