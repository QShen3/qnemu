/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

enum class GbInterrupt : uint8_t
{
    vBlank = 0x01,
    lcd = 0x02,
    timer = 0x04,
    serial = 0x08,
    joypad = 0x10,
};

/**
 * IF/IE register pair.  This class only records requests; deciding whether an
 * interrupt is actually taken is the CPU's job and belongs to its instruction
 * boundary logic.
 */
class GbInterruptHandlerInterface : public GbDeviceInterface
{
public:
    ~GbInterruptHandlerInterface() override = default;

    virtual void request(GbInterrupt interrupt) = 0;

    /// IF & IE & 0x1F, i.e. the interrupts currently waiting to be serviced.
    virtual uint8_t pending() const = 0;

    /// Clear the given bits in IF (normally called by the CPU dispatch).
    virtual void acknowledge(uint8_t mask) = 0;

    virtual void requestVBlankInterrupt() = 0;
    virtual void requestLcdInterrupt() = 0;
    virtual void requestTimerInterrupt() = 0;
    virtual void requestSerialInterrupt() = 0;
    virtual void requestJoypadInterrupt() = 0;
};

}  // namespace qnemu
