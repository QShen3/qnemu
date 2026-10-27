/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel4.h"

namespace qnemu
{

GbChannel4::GbChannel4()
{
    reset();
}

uint8_t GbChannel4::read(uint16_t address) const
{
    switch (address) {
    case 0xFF20:
        return 0xFF;
    case 0xFF21:
        return nr42;
    case 0xFF22:
        return nr43;
    case 0xFF23:
        return static_cast<uint8_t>(0xBF | (nr44 & 0x40));
    default:
        return 0xFF;
    }
}

void GbChannel4::write(uint16_t address, const uint8_t& value)
{
    switch (address) {
    case 0xFF20:
        nr41 = static_cast<uint8_t>(value & 0x3F);
        lengthTimer.setLength(nr41);
        break;
    case 0xFF21:
        nr42 = value;
        envelope.configure(value);
        if (!envelope.isDacEnabled()) {
            enabled = false;
            data = 0;
        }
        break;
    case 0xFF22:
        nr43 = value;
        break;
    case 0xFF23:
        nr44 = static_cast<uint8_t>(value & 0xC0);
        lengthTimer.setEnabled((nr44 & 0x40) != 0);
        if ((nr44 & 0x80) != 0) {
            trigger();
        }
        break;
    default:
        break;
    }
}

void GbChannel4::trigger()
{
    if (!envelope.isDacEnabled()) {
        return;
    }

    enabled = true;
    lengthTimer.trigger();
    envelope.trigger();

    // The hardware clears the LFSR on trigger; the first shifted-out bit is
    // therefore 0 and the channel starts silent for a few clocks.
    lfsr = 0;
    timer = clockPeriod();
    data = 0;
}

uint32_t GbChannel4::clockPeriod() const
{
    const uint8_t shift = static_cast<uint8_t>((nr43 >> 4) & 0x0F);
    if (shift >= 14) {
        // A shift of 14 or 15 stops the LFSR clock entirely.
        return 0;
    }

    const uint8_t divisor = static_cast<uint8_t>(nr43 & 0x07);
    const uint32_t base = divisor == 0 ? 8 : static_cast<uint32_t>(divisor) * 16;
    return base << shift;
}

void GbChannel4::stepLfsr()
{
    // Feedback is the XNOR of bits 0 and 1, written into the temporary bit.
    const uint16_t feedback = ((lfsr & 0x01) == ((lfsr >> 1) & 0x01)) ? 1 : 0;
    lfsr |= static_cast<uint16_t>(feedback << 15);
    if ((nr43 & 0x08) != 0) {
        lfsr = static_cast<uint16_t>((lfsr & ~0x0080) | (feedback << 7));
    }
    lfsr >>= 1;
}

void GbChannel4::step()
{
    if (!enabled) {
        data = 0;
        return;
    }

    const uint8_t shift = static_cast<uint8_t>((nr43 >> 4) & 0x0F);
    if (shift < 14) {
        if (timer > 0) {
            --timer;
        }
        if (timer == 0) {
            timer = clockPeriod();
            stepLfsr();
        }
    }

    data = (lfsr & 0x01) != 0 ? envelope.getVolume() : 0;
}

void GbChannel4::clockLength()
{
    lengthTimer.clock();
    if (lengthTimer.isEnabled() && lengthTimer.expired()) {
        enabled = false;
        data = 0;
    }
}

void GbChannel4::clockEnvelope()
{
    envelope.clock();
}

void GbChannel4::reset()
{
    nr41 = 0;
    nr42 = 0;
    nr43 = 0;
    nr44 = 0;

    enabled = false;
    timer = 0;
    lfsr = 0;
    data = 0;

    lengthTimer.reset();
    envelope.reset();
}

}  // namespace qnemu
