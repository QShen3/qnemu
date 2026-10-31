/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>

#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbMmuInterface.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

class GbMmu : public GbMmuInterface
{
public:
    GbMmu() = delete;
    GbMmu(std::unique_ptr<GbApu> apu,
        std::shared_ptr<GbCartridgeInterface> cartridge,
        std::unique_ptr<GbGpuInterface> gpu,
        std::unique_ptr<GbHighRam> highRam,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::unique_ptr<GbJoypad> joypad,
        std::unique_ptr<GbWorkRam> workRam,
        std::unique_ptr<GbTimer> timer);
    ~GbMmu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
    void setDoubleSpeed(bool doubleSpeed) override;
    void setCgb(bool cgb) override;
    void setDmgCompatibilityMode(bool enabled) override;
    void triggerOamBugWrite(uint16_t address, int rowOffset = 0) override;
    void triggerOamBugRead(uint16_t address) override;
    void triggerOamBugReadIncrease(uint16_t address) override;

    /// Boot ROM mapping.  The image uses the standard CGB layout, so the
    /// 0x0100-0x01FF cartridge-header gap is never served from it.
    void setBootRom(std::span<const uint8_t> rom) { bootRom = rom; }
    void setBootRomActive(bool active) { bootRomActive = active; }
    bool isBootRomActive() const { return bootRomActive; }

private:
    std::unique_ptr<GbApu> apu;
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::unique_ptr<GbGpuInterface> gpu;
    std::unique_ptr<GbHighRam> highRam;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    std::unique_ptr<GbJoypad> joypad;
    std::unique_ptr<GbWorkRam> workRam;
    std::unique_ptr<GbTimer> timer;
    std::span<const uint8_t> bootRom;
    bool bootRomActive = false;
    bool cgbHardware = false;
    bool dmgMode = false;
    std::array<uint8_t, 4> cgbUnmappedRegisters {};
};

}  // namespace qnemu
