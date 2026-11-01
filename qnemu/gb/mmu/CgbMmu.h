/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>

#include "qnemu/gb/memory/CgbWorkRam.h"
#include "qnemu/gb/mmu/DmgMmu.h"
#include "qnemu/gb/infrared/GbInfrared.h"

namespace qnemu
{

/**
 * CGB address decoding.
 *
 * Extends the DMG map with the CGB-only I/O registers.  The CGB boot ROM may
 * switch the machine into DMG compatibility through KEY0; the CGB hardware
 * registers remain visible while WRAM banking follows DMG behavior.
 */
class CgbMmu final : public DmgMmu
{
public:
    CgbMmu(std::unique_ptr<GbApu> apu,
        std::shared_ptr<GbCartridgeInterface> cartridge,
        std::unique_ptr<GbGpuInterface> gpu,
        std::unique_ptr<GbHighRam> highRam,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::unique_ptr<GbJoypad> joypad,
        std::unique_ptr<CgbWorkRam> workRam,
        std::unique_ptr<GbTimer> timer,
        std::unique_ptr<GbSerial> serial,
        std::unique_ptr<GbInfrared> infrared);
    ~CgbMmu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    void setCgb(bool cgb) override;
    void setDmgCompatibilityMode(bool enabled) override;
    void setDoubleSpeed(bool doubleSpeed) override;

private:
    CgbWorkRam& cgbWorkRam() const;

    std::array<uint8_t, 4> cgbUnmappedRegisters_ {};
    std::unique_ptr<GbInfrared> infrared_;
    bool dmgMode_ = false;
};

}  // namespace qnemu
