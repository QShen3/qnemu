/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

GbTimer::GbTimer(std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) :
    interruptHandler(std::move(interruptHandler))
{
    GbTimer::reset();
}

uint8_t GbTimer::read(uint16_t address) const
{
    switch (address) {
    case 0xFF04:
        return static_cast<uint8_t>(divider >> 8);
    case 0xFF05:
        return timerCounter;
    case 0xFF06:
        return timerModulo;
    case 0xFF07:
        return static_cast<uint8_t>(timerControl | 0xF8);
    default:
        assert(false && "Wrong address");
        return 0xFF;
    }
}

void GbTimer::write(uint16_t address, const uint8_t& value)
{
    switch (address) {
    case 0xFF04: {
        // Resetting the divider can create a falling edge on both the timer's
        // selected bit and the DIV-APU bit.
        if ((timerControl & 0x04) != 0 && selectedClockBit() != 0) {
            incrementTimerCounter();
        }
        if (((divider >> divApuBitIndex()) & 0x01) != 0) {
            divApuTickPending = true;
        }

        divider = 0;
        previousTimerBit = 0;
        previousDivApuBit = 0;
        break;
    }
    case 0xFF05:
        if (overflow && overflowCycles == 0) {
            // Writing TIMA on the overflow cycle suppresses the reload and
            // the interrupt request.
            timerCounter = value;
            overflow = false;
            overflowCycles = 0;
            reloadWindowCycles = 0;
        } else if (overflow || reloadWindowCycles > 0) {
            // During the reload M-cycle the TMA value wins.
            timerCounter = timerModulo;
        } else {
            timerCounter = value;
        }
        break;
    case 0xFF06:
        timerModulo = value;
        if (overflow || reloadWindowCycles > 0) {
            // A TMA write during the reload M-cycle is copied to TIMA too.
            timerCounter = timerModulo;
        }
        break;
    case 0xFF07: {
        const bool oldEnabled = (timerControl & 0x04) != 0;
        const uint8_t oldBit = selectedClockBit();
        const uint8_t oldSelect = static_cast<uint8_t>(timerControl & 0x03);

        timerControl = value;

        const bool newEnabled = (timerControl & 0x04) != 0;
        const uint8_t newBit = selectedClockBit();
        const uint8_t newSelect = static_cast<uint8_t>(timerControl & 0x03);

        bool tick = false;
        if (oldEnabled && oldBit != 0) {
            if (!newEnabled) {
                // Disabling while the selected bit is high emits a tick.
                tick = true;
            } else if (oldSelect != newSelect && newBit == 0) {
                // Changing from a set bit to an unset one also ticks.
                tick = true;
            }
        }
        if (tick) {
            incrementTimerCounter();
        }

        previousTimerBit = newEnabled ? newBit : 0;
        break;
    }
    default:
        assert(false && "Wrong address");
        break;
    }
}

void GbTimer::step()
{
    tick();
    if (doubleSpeed) {
        tick();
    }
}

void GbTimer::reset()
{
    divider = 0xAB00;
    timerCounter = 0;
    timerModulo = 0;
    timerControl = 0xF8;

    overflow = false;
    overflowCycles = 0;
    reloadWindowCycles = 0;
    previousTimerBit = 0;
    previousDivApuBit = static_cast<uint8_t>((divider >> divApuBitIndex()) & 0x01);
    divApuTickPending = false;
}

void GbTimer::setDoubleSpeed(bool value)
{
    if (doubleSpeed == value) {
        return;
    }
    doubleSpeed = value;

    // Update the edge-detector state so the speed switch itself does not
    // produce a spurious timer or DIV-APU tick.
    previousDivApuBit = static_cast<uint8_t>((divider >> divApuBitIndex()) & 0x01);
    previousTimerBit = (timerControl & 0x04) != 0 ? selectedClockBit() : 0;
}

void GbTimer::setDivider(uint16_t value)
{
    divider = value;
    previousTimerBit = (timerControl & 0x04) != 0 ? selectedClockBit() : 0;
    previousDivApuBit = static_cast<uint8_t>((divider >> divApuBitIndex()) & 0x01);
    divApuTickPending = false;
}

bool GbTimer::takeDivApuTick()
{
    if (!divApuTickPending) {
        return false;
    }
    divApuTickPending = false;
    return true;
}

bool GbTimer::isDivApuBitHigh() const
{
    return ((divider >> divApuBitIndex()) & 0x01) != 0;
}

uint8_t GbTimer::selectedClockBitIndex() const
{
    switch (timerControl & 0x03) {
    case 0:
        return 9;
    case 1:
        return 3;
    case 2:
        return 5;
    default:
        return 7;
    }
}

uint8_t GbTimer::selectedClockBit() const
{
    return static_cast<uint8_t>((divider >> selectedClockBitIndex()) & 0x01);
}

uint8_t GbTimer::divApuBitIndex() const
{
    return doubleSpeed ? 13 : 12;
}

void GbTimer::tick()
{
    ++divider;
    updateDivApuBit();

    if (reloadWindowCycles > 0) {
        --reloadWindowCycles;
    }

    if (overflow) {
        ++overflowCycles;
        if (overflowCycles >= 4) {
            timerCounter = timerModulo;
            overflow = false;
            overflowCycles = 0;
            reloadWindowCycles = 4;
            interruptHandler->requestTimerInterrupt();
        }
    }

    if ((timerControl & 0x04) == 0) {
        return;
    }

    const uint8_t bit = selectedClockBit();
    if (previousTimerBit != 0 && bit == 0) {
        incrementTimerCounter();
    }
    previousTimerBit = bit;
}

void GbTimer::incrementTimerCounter()
{
    if (timerCounter == 0xFF) {
        timerCounter = 0;
        overflow = true;
        overflowCycles = 0;
        reloadWindowCycles = 0;
    } else {
        ++timerCounter;
    }
}

void GbTimer::updateDivApuBit()
{
    const uint8_t bit = static_cast<uint8_t>((divider >> divApuBitIndex()) & 0x01);
    if (previousDivApuBit != 0 && bit == 0) {
        divApuTickPending = true;
    }
    previousDivApuBit = bit;
}

}  // namespace qnemu
