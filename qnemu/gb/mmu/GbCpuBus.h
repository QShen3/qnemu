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
 * the SM83 machine cycle.  CGB speed switching is kept here as well because it
 * changes how many peripheral dots one CPU T-cycle represents.
 */
class GbCpuBus : public CpuBus
{
public:
    explicit GbCpuBus(GbMmuInterface& mmu, bool cgb = false) : mmu(mmu), cgb(cgb) {}
    ~GbCpuBus() override = default;

    void setCgb(bool value) { cgb = value; }

    uint8_t read(uint16_t address) override
    {
        if (address == 0xFF4C) {
            return cgb ? 0x80 : 0xFF;
        }
        if (address == 0xFF4D) {
            return static_cast<uint8_t>(0x7E
                | (doubleSpeed ? 0x80 : 0x00)
                | (speedSwitchArmed ? 0x01 : 0x00));
        }
        return mmu.read(address);
    }

    void write(uint16_t address, uint8_t value) override
    {
        if (address == 0xFF4C) {
            return;
        }
        if (address == 0xFF4D) {
            if (cgb && (value & 0x01) != 0) {
                speedSwitchArmed = true;
            }
            return;
        }
        mmu.write(address, value);
    }

    void tick(uint8_t cycles) override
    {
        uint8_t peripheralCycles = cycles;
        if (doubleSpeed) {
            const uint16_t total = static_cast<uint16_t>(cycles + speedRemainder);
            peripheralCycles = static_cast<uint8_t>(total / 2);
            speedRemainder = static_cast<uint8_t>(total & 0x01);
        } else {
            speedRemainder = 0;
        }

        for (uint8_t i = 0; i < peripheralCycles; ++i) {
            mmu.step();
        }
    }

    bool onStop() override
    {
        if (speedSwitchArmed) {
            speedSwitchArmed = false;
            doubleSpeed = !doubleSpeed;
            speedRemainder = 0;
            mmu.setDoubleSpeed(doubleSpeed);
            return false;
        }
        return true;
    }

private:
    GbMmuInterface& mmu;
    bool cgb = false;
    bool doubleSpeed = false;
    bool speedSwitchArmed = false;
    uint8_t speedRemainder = 0;
};

}  // namespace qnemu
