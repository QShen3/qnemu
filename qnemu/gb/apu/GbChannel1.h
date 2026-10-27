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
 * Channel 1: pulse/square wave with a frequency sweep unit.
 *
 * Registers: NR10 (sweep), NR11 (duty + length), NR12 (volume + envelope),
 * NR13/NR14 (period + control).
 */
class GbChannel1 : public GbChannelInterface
{
public:
    GbChannel1();
    ~GbChannel1() override = default;

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
    void clockSweep() override;
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
        return static_cast<uint16_t>(((nr14 & 0x07) << 8) | nr13);
    }
    void trigger();
    void triggerSweep();
    uint16_t calculateSweepPeriod() const;

    uint8_t nr10 = 0;  // FF10
    uint8_t nr11 = 0;  // FF11
    uint8_t nr12 = 0;  // FF12
    uint8_t nr13 = 0;  // FF13
    uint8_t nr14 = 0;  // FF14

    bool enabled = false;
    bool nextStepClocksLength = true;
    bool sweepNegateUsed = false;
    uint16_t frequencyTimer = 0;
    uint8_t dutyPosition = 0;
    uint8_t data = 0;

    GbLengthTimer lengthTimer{64};
    GbVolumeEnvelope envelope;

    uint16_t shadowPeriod = 0;
    uint8_t sweepTimer = 0;
    bool sweepEnabled = false;
};

}  // namespace qnemu
