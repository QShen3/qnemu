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
    GbInterruptHandler::reset();
}

uint8_t GbInterruptHandler::read(uint16_t address) const
{
    if (address == 0xFF0F) {
        // IF bits 5-7 are unused and read as 1.
        return static_cast<uint8_t>(interruptFlag.load() | 0xE0);
    }
    if (address == 0xFFFF) {
        return interruptEnabled.load();
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbInterruptHandler::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF0F) {
        interruptFlag.store(value);
        return;
    }
    if (address == 0xFFFF) {
        interruptEnabled.store(value);
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
    interruptFlag.store(0xE1);
    interruptEnabled.store(0x00);
}

void GbInterruptHandler::request(GbInterrupt interrupt)
{
    interruptFlag.fetch_or(static_cast<uint8_t>(interrupt));
}

uint8_t GbInterruptHandler::pending() const
{
    return static_cast<uint8_t>(interruptFlag.load() & interruptEnabled.load() & 0x1F);
}

void GbInterruptHandler::acknowledge(uint8_t mask)
{
    interruptFlag.fetch_and(static_cast<uint8_t>(~mask));
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
