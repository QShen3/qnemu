/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemu
{

GbInterruptHandler::GbInterruptHandler()
{
    reset();
}

uint8_t GbInterruptHandler::read(uint16_t address) const
{
    if (address == 0xFF0F) {
        return interruptFlag;
    }
    if (address == 0xFFFF) {
        return interruptEnabled;
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbInterruptHandler::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF0F) {
        interruptFlag = value;
        return;
    }
    if (address == 0xFFFF) {
        interruptEnabled = value;
        return;
    }

    assert(false && "Wrong address");
}

void GbInterruptHandler::step()
{
    // Interrupt dispatch is handled by the CPU at instruction boundaries.
}

void GbInterruptHandler::reset()
{
    interruptFlag = 0xE1;
    interruptEnabled = 0x00;
}

void GbInterruptHandler::request(GbInterrupt interrupt)
{
    interruptFlag = static_cast<uint8_t>(interruptFlag | static_cast<uint8_t>(interrupt));
}

uint8_t GbInterruptHandler::pending() const
{
    return static_cast<uint8_t>(interruptFlag & interruptEnabled & 0x1F);
}

void GbInterruptHandler::acknowledge(uint8_t mask)
{
    interruptFlag = static_cast<uint8_t>(interruptFlag & static_cast<uint8_t>(~mask));
}

void GbInterruptHandler::requestVBlankInterrupt()
{
    request(GbInterrupt::vBlank);
}

void GbInterruptHandler::requestLcdInterrupt()
{
    request(GbInterrupt::lcd);
}

void GbInterruptHandler::requestTimerInterrupt()
{
    request(GbInterrupt::timer);
}

void GbInterruptHandler::requestSerialInterrupt()
{
    request(GbInterrupt::serial);
}

void GbInterruptHandler::requestJoypadInterrupt()
{
    request(GbInterrupt::joypad);
}

}  // namespace qnemu
