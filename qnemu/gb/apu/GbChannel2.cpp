/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel2.h"

namespace qnemu
{

namespace
{

// Same duty patterns as channel 1, most significant bit played first.
constexpr uint8_t DutyPatterns[4] = {0x01, 0x81, 0x87, 0x7E};

uint8_t dutyBit(uint8_t duty, uint8_t position)
{
    return static_cast<uint8_t>((DutyPatterns[duty & 0x03] >> (7 - (position & 0x07))) & 0x01);
}

}  // namespace

GbChannel2::GbChannel2()
{
    reset();
}

uint8_t GbChannel2::read(uint16_t address) const
{
    switch (address) {
    case 0xFF16:
        return static_cast<uint8_t>(0x3F | (nr21 & 0xC0));
    case 0xFF17:
        return nr22;
    case 0xFF18:
        return 0xFF;
    case 0xFF19:
        return static_cast<uint8_t>(0xBF | (nr24 & 0x40));
    default:
        return 0xFF;
    }
}

void GbChannel2::write(uint16_t address, const uint8_t& value)
{
    switch (address) {
    case 0xFF16:
        nr21 = static_cast<uint8_t>(value & 0xC0);
        lengthTimer.setLength(static_cast<uint8_t>(value & 0x3F));
        break;
    case 0xFF17:
        nr22 = value;
        envelope.configure(value);
        if (!envelope.isDacEnabled()) {
            enabled = false;
            data = 0;
        }
        break;
    case 0xFF18:
        nr23 = value;
        break;
    case 0xFF19:
        nr24 = static_cast<uint8_t>(value & 0xC7);
        lengthTimer.setEnabled((nr24 & 0x40) != 0);
        if ((nr24 & 0x80) != 0) {
            trigger();
        }
        break;
    default:
        break;
    }
}

void GbChannel2::trigger()
{
    if (!envelope.isDacEnabled()) {
        return;
    }

    enabled = true;
    lengthTimer.trigger();
    frequencyTimer = static_cast<uint16_t>((2048 - period()) * 4);
    envelope.trigger();
}

void GbChannel2::step()
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

    data = dutyBit(static_cast<uint8_t>(nr21 >> 6), dutyPosition) != 0
        ? envelope.getVolume()
        : 0;
}

void GbChannel2::clockLength()
{
    lengthTimer.clock();
    if (lengthTimer.isEnabled() && lengthTimer.expired()) {
        enabled = false;
        data = 0;
    }
}

void GbChannel2::clockEnvelope()
{
    envelope.clock();
}

void GbChannel2::reset()
{
    nr21 = 0;
    nr22 = 0;
    nr23 = 0;
    nr24 = 0;

    enabled = false;
    frequencyTimer = 0;
    dutyPosition = 0;
    data = 0;

    lengthTimer.reset();
    envelope.reset();
}

}  // namespace qnemu
