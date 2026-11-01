/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <functional>
#include <utility>

#include "qnemu/gb/mmu/DmgCpuBus.h"

namespace qnemu
{

/**
 * CGB CPU bus adapter.
 *
 * Adds KEY0/KEY1 and the double-speed clock.  The CGB hardware registers
 * themselves are decoded by CgbMmu; this class only owns the CPU-visible
 * speed-switch state.
 */
class CgbCpuBus : public DmgCpuBus
{
public:
    explicit CgbCpuBus(GbMmuInterface& mmu) : DmgCpuBus(mmu) {}
    ~CgbCpuBus() override = default;

    void setCgb(bool value)
    {
        cgb = value;
        mmu.setCgb(value);
    }

    /// Called when the CGB boot ROM switches the CPU to DMG compatibility via
    /// KEY0.  The complete system uses this to switch the PPU renderer too.
    void setCgbModeCallback(std::function<void(bool)> callback)
    {
        cgbModeCallback = std::move(callback);
    }

    /// Returns the bus to normal speed.  Used when a cartridge is (re)loaded.
    void reset()
    {
        doubleSpeed = false;
        speedSwitchArmed = false;
        speedRemainder = 0;
        key0Locked = false;
        mmu.setDoubleSpeed(false);
    }

    uint8_t read(uint16_t address) override
    {
        if (address == 0xFF4C) {
            return cgb ? 0x80 : 0xFF;
        }
        if (address == 0xFF4D) {
            if (!cgb) {
                return 0xFF;
            }
            return static_cast<uint8_t>(0x7E
                | (doubleSpeed ? 0x80 : 0x00)
                | (speedSwitchArmed ? 0x01 : 0x00));
        }
        return DmgCpuBus::read(address);
    }

    void write(uint16_t address, uint8_t value) override
    {
        if (address == 0xFF50) {
            key0Locked = true;
            DmgCpuBus::write(address, value);
            return;
        }
        if (address == 0xFF4C) {
            if (cgb && !key0Locked && (value & 0x04) != 0) {
                cgb = false;
                mmu.setDmgCompatibilityMode(true);
                if (cgbModeCallback) {
                    cgbModeCallback(false);
                }
            }
            return;
        }
        if (address == 0xFF4D) {
            if (cgb && (value & 0x01) != 0) {
                speedSwitchArmed = true;
            }
            return;
        }
        DmgCpuBus::write(address, value);
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
    bool cgb = true;
    bool doubleSpeed = false;
    bool speedSwitchArmed = false;
    bool key0Locked = false;
    uint8_t speedRemainder = 0;
    std::function<void(bool)> cgbModeCallback;
};

}  // namespace qnemu
