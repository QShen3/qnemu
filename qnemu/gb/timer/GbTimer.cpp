/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <memory>

#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

GbTimer::GbTimer(std::shared_ptr<GbInterruptHandler> interruptHandler) : interruptHandler(interruptHandler)
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
        return registers.divider;
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

}

void GbTimer::reset()
{
    registers.divider = 0xAB;
    registers.timerCounter = 0;
    registers.timerModulo = 0;
    registers.timerControl = 0xF8;
}

}  // namespace qnemu