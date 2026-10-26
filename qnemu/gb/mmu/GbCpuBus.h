/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/cpu/CpuBus.h"
#include "qnemu/gb/mmu/GbMmuInterface.h"

namespace qnemu
{

/**
 * Adapts the 1 T-cycle GbMmu::step() interface to the CPU's M-cycle bus.
 * One CPU memory access advances every peripheral by four T-cycles, matching
 * the SM83 machine cycle.
 */
class GbCpuBus : public CpuBus
{
public:
    explicit GbCpuBus(GbMmuInterface& mmu) : mmu(mmu) {}
    ~GbCpuBus() override = default;

    uint8_t read(uint16_t address) override
    {
        return mmu.read(address);
    }

    void write(uint16_t address, uint8_t value) override
    {
        mmu.write(address, value);
    }

    void tick(uint8_t cycles) override
    {
        for (uint8_t i = 0; i < cycles; ++i) {
            mmu.step();
        }
    }

private:
    GbMmuInterface& mmu;
};

}  // namespace qnemu
