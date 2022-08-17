/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <memory>

#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

GbTimer::GbTimer(std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) : interruptHandler(interruptHandler)
{
    GbTimer::reset();
}

bool GbTimer::accepts(uint16_t address) const
{
    if (address >= 0xFF04 && address <= 0xFF07) {
        return true;
    }
    return false;
}

uint8_t GbTimer::read(uint16_t address) const
{
    if (address == 0xFF04) {
        return registers.divider >> 8;
    }
    if (address == 0xFF05) {
        return registers.timerCounter;
    }
    if (address == 0xFF06) {
        return registers.timerModulo;
    }
    if (address == 0xFF07) {
        return registers.timerControl;
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbTimer::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF04) {
        registers.divider = 0;
        return;
    } 
    if (address == 0xFF05) {
        registers.timerCounter = value;
        return;
    }
    if (address == 0xFF06) {
        registers.timerModulo = value;
        return;
    }
    if (address == 0xFF07) {
        registers.timerControl = value;
        return;
    }

    assert(false && "Wrong address");
}

void GbTimer::step()
{
    registers.divider++;

    if (overflow) {
        ticksSinceOverflow++;
        if (ticksSinceOverflow == 4) {
            interruptHandler->requestTimerInterrupt();
        } else if (ticksSinceOverflow == 5) {
            registers.timerCounter = registers.timerModulo;
        } else if (ticksSinceOverflow == 6) {
            registers.timerCounter = registers.timerModulo;
            overflow = false;
            ticksSinceOverflow = 0;
        }
    }

    if (!registers.timerEnable) {
        return;
    }
    uint8_t bit = 0;
    if (registers.inputClockSelect == 0) {
        bit = (registers.divider & 1024) >> 10;
    } else if (registers.inputClockSelect == 1) {
        bit = (registers.divider & 16) >> 4;
    } else if (registers.inputClockSelect == 2) {
        bit = (registers.divider & 64) >> 6;
    } else if (registers.inputClockSelect == 3) {
        bit = (registers.divider & 256) >> 8;
    }

    if (bit != previousBit) {
        if (registers.timerCounter == 0xFF) {
            overflow = true;
        }
        registers.timerCounter++;
        previousBit = bit;
    }
}

void GbTimer::reset()
{
    registers.divider = 0xAB00;
    registers.timerCounter = 0;
    registers.timerModulo = 0;
    registers.timerControl = 0xF8;

    overflow = false;
    previousBit = 0;
    ticksSinceOverflow = 0;
}

}  // namespace qnemu