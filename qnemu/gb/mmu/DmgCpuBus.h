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
 * DMG CPU bus adapter.
 *
 * One CPU memory access advances every peripheral by four T-cycles, matching
 * the SM83 machine cycle.  The DMG has no KEY1/double-speed state.
 */
class DmgCpuBus : public CpuBus
{
public:
    explicit DmgCpuBus(GbMmuInterface& mmu) : mmu(mmu) {}
    ~DmgCpuBus() override = default;

    /// Returns the bus to its power-on state.  Present for parity with the
    /// CGB bus, which has speed-switch state to clear.
    void reset() {}

    uint8_t read(uint16_t address) override { return mmu.read(address); }
    void write(uint16_t address, uint8_t value) override { mmu.write(address, value); }
    bool isStalled() const override { return mmu.isCpuStalled(); }

    void tick(uint8_t cycles) override
    {
        for (uint8_t i = 0; i < cycles; ++i) {
            mmu.step();
        }
    }

    void triggerOamBugWrite(uint16_t address, int rowOffset = 0) override
    {
        mmu.triggerOamBugWrite(address, rowOffset);
    }

    void triggerOamBugRead(uint16_t address) override
    {
        mmu.triggerOamBugRead(address);
    }

    void triggerOamBugReadIncrease(uint16_t address) override
    {
        mmu.triggerOamBugReadIncrease(address);
    }

    bool onStop() override { return true; }

protected:
    GbMmuInterface& mmu;
};

}  // namespace qnemu
