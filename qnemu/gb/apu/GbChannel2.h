/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/apu/GbLengthTimer.h"
#include "qnemu/gb/apu/GbVolumeEnvelope.h"

namespace qnemu
{

/**
 * Channel 2: pulse/square wave.  Identical to channel 1 without the sweep
 * unit.  Registers: NR21..NR24 (FF16..FF19).
 */
class GbChannel2 : public GbChannelInterface
{
public:
    GbChannel2();
    ~GbChannel2() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
    void powerOff(bool preserveLength = false) override;

    uint8_t getData() const override { return data; }
    bool isEnabled() const override { return enabled; }
    bool isDacEnabled() const override { return envelope.isDacEnabled(); }

    void clockLength() override;
    void clockEnvelope() override;
    void setLengthClockPhase(bool nextStepClocksLength) override
    {
        this->nextStepClocksLength = nextStepClocksLength;
    }
    void writeLengthWhileOff(uint8_t value) override
    {
        lengthTimer.setLength(static_cast<uint8_t>(value & 0x3F));
    }

    /// Current 11-bit period value (exposed for tests and diagnostics).
    uint16_t getPeriod() const { return period(); }

private:
    uint16_t period() const
    {
        return static_cast<uint16_t>(((nr24 & 0x07) << 8) | nr23);
    }
    void trigger();

    uint8_t nr21 = 0;  // FF16
    uint8_t nr22 = 0;  // FF17
    uint8_t nr23 = 0;  // FF18
    uint8_t nr24 = 0;  // FF19

    bool enabled = false;
    bool nextStepClocksLength = true;
    uint16_t frequencyTimer = 0;
    uint8_t dutyPosition = 0;
    uint8_t data = 0;

    GbLengthTimer lengthTimer{64};
    GbVolumeEnvelope envelope;
};

}  // namespace qnemu
