/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel1.h"

namespace qnemu
{

namespace
{

// Duty patterns, most significant bit played first.  All patterns start with
// a 0, matching the hardware's "first pulse step outputs zero" behaviour.
constexpr uint8_t DutyPatterns[4] = {0x01, 0x81, 0x87, 0x7E};

uint8_t dutyBit(uint8_t duty, uint8_t position)
{
    return static_cast<uint8_t>((DutyPatterns[duty & 0x03] >> (7 - (position & 0x07))) & 0x01);
}

}  // namespace

GbChannel1::GbChannel1()
{
    reset();
}

uint8_t GbChannel1::read(uint16_t address) const
{
    switch (address) {
    case 0xFF10:
        return static_cast<uint8_t>(0x80 | (nr10 & 0x7F));
    case 0xFF11:
        return static_cast<uint8_t>(0x3F | (nr11 & 0xC0));
    case 0xFF12:
        return nr12;
    case 0xFF13:
        return 0xFF;
    case 0xFF14:
        return static_cast<uint8_t>(0xBF | (nr14 & 0x40));
    default:
        return 0xFF;
    }
}

void GbChannel1::write(uint16_t address, const uint8_t& value)
{
    switch (address) {
    case 0xFF10:
        nr10 = static_cast<uint8_t>(value & 0x7F);
        break;
    case 0xFF11:
        nr11 = static_cast<uint8_t>(value & 0xC0);
        lengthTimer.setLength(static_cast<uint8_t>(value & 0x3F));
        break;
    case 0xFF12:
        nr12 = value;
        envelope.configure(value);
        if (!envelope.isDacEnabled()) {
            enabled = false;
            data = 0;
        }
        break;
    case 0xFF13:
        nr13 = value;
        break;
    case 0xFF14:
        nr14 = static_cast<uint8_t>(value & 0xC7);
        lengthTimer.setEnabled((nr14 & 0x40) != 0);
        if ((nr14 & 0x80) != 0) {
            trigger();
        }
        break;
    default:
        break;
    }
}

void GbChannel1::trigger()
{
    if (!envelope.isDacEnabled()) {
        return;
    }

    enabled = true;
    lengthTimer.trigger();
    frequencyTimer = static_cast<uint16_t>((2048 - period()) * 4);
    envelope.trigger();
    triggerSweep();
}

void GbChannel1::triggerSweep()
{
    shadowPeriod = period();
    sweepTimer = static_cast<uint8_t>((nr10 >> 4) & 0x07);
    sweepEnabled = sweepTimer != 0 || (nr10 & 0x07) != 0;

    // The overflow check also runs when the sweep pace is 0, as long as the
    // individual step is non-zero.
    if ((nr10 & 0x07) != 0 && calculateSweepPeriod() > 0x7FF) {
        enabled = false;
    }
}

uint16_t GbChannel1::calculateSweepPeriod() const
{
    const uint16_t delta = static_cast<uint16_t>(shadowPeriod >> (nr10 & 0x07));
    if ((nr10 & 0x08) != 0) {
        return static_cast<uint16_t>(shadowPeriod - delta);
    }
    return static_cast<uint16_t>(shadowPeriod + delta);
}

void GbChannel1::clockSweep()
{
    if (!sweepEnabled) {
        return;
    }

    if (sweepTimer > 0) {
        --sweepTimer;
    }
    if (sweepTimer > 0) {
        return;
    }

    const uint8_t pace = static_cast<uint8_t>((nr10 >> 4) & 0x07);
    if (pace == 0) {
        return;
    }
    sweepTimer = pace;

    const uint16_t newPeriod = calculateSweepPeriod();
    if (newPeriod > 0x7FF) {
        enabled = false;
        return;
    }

    if ((nr10 & 0x07) != 0) {
        shadowPeriod = newPeriod;
        nr13 = static_cast<uint8_t>(newPeriod & 0xFF);
        nr14 = static_cast<uint8_t>((nr14 & 0xF8) | ((newPeriod >> 8) & 0x07));

        // The hardware runs the calculation and overflow check a second time.
        if (calculateSweepPeriod() > 0x7FF) {
            enabled = false;
        }
    }
}

void GbChannel1::step()
{
    if (!enabled) {
        data = 0;
        return;
    }

    if (frequencyTimer > 0) {
        --frequencyTimer;
    }
    if (frequencyTimer == 0) {
        frequencyTimer = static_cast<uint16_t>((2048 - period()) * 4);
        dutyPosition = static_cast<uint8_t>((dutyPosition + 1) & 0x07);
    }

    data = dutyBit(static_cast<uint8_t>(nr11 >> 6), dutyPosition) != 0
        ? envelope.getVolume()
        : 0;
}

void GbChannel1::clockLength()
{
    lengthTimer.clock();
    if (lengthTimer.isEnabled() && lengthTimer.expired()) {
        enabled = false;
        data = 0;
    }
}

void GbChannel1::clockEnvelope()
{
    envelope.clock();
}

void GbChannel1::reset()
{
    nr10 = 0;
    nr11 = 0;
    nr12 = 0;
    nr13 = 0;
    nr14 = 0;

    enabled = false;
    frequencyTimer = 0;
    dutyPosition = 0;
    data = 0;

    lengthTimer.reset();
    envelope.reset();

    shadowPeriod = 0;
    sweepTimer = 0;
    sweepEnabled = false;
}

}  // namespace qnemu
