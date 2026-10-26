/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/cpu/CpuBus.h"

namespace qnemuTest
{

/**
 * Flat 64 KiB bus for CPU unit tests.  It deliberately has no timer, PPU or
 * MMU: tests that need those behaviours use the headless ROM runner instead.
 */
class TestBus : public qnemu::CpuBus
{
public:
    uint8_t read(uint16_t address) override
    {
        return memory.at(address);
    }

    void write(uint16_t address, uint8_t value) override
    {
        memory.at(address) = value;
    }

    void tick(uint8_t cycles) override
    {
        ticks += cycles;
    }

    std::array<uint8_t, 0x10000> memory{};
    uint64_t ticks = 0;
};

}  // namespace qnemuTest
