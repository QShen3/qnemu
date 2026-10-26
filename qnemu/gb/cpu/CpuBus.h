/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

/**
 * The only thing the SM83 core knows about the outside world.
 *
 * Every memory access is one machine cycle (4 T-cycles), so the core asks the
 * bus to advance the rest of the machine by exactly the cycles the access
 * takes.  Keeping this interface tiny makes the CPU testable with a plain
 * array and makes the timing visible in the instruction implementations.
 */
class CpuBus
{
public:
    virtual ~CpuBus() = default;

    virtual uint8_t read(uint16_t address) = 0;
    virtual void write(uint16_t address, uint8_t value) = 0;
    virtual void tick(uint8_t cycles) = 0;

    /**
     * Called after the STOP opcode and its padding byte have been fetched.
     * Return true to enter the low-power stopped state (the DMG behaviour),
     * or false when the system completed the STOP immediately (for example a
     * CGB KEY1 speed switch).  The default keeps the CPU stopped.
     */
    virtual bool onStop() { return true; }
};

}  // namespace qnemu
