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
#include "qnemu/gb/memory/DmgWorkRam.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/mmu/GbMmuInterface.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

/**
 * DMG address decoding.
 *
 * This class only implements the original Game Boy map.  CGB-only registers
 * (KEY1 is on the CPU bus, VRAM bank, HDMA, palette RAM, SVBK and the
 * undocumented FF72-FF75 registers) are handled by CgbMmu.
 */
class DmgMmu : public GbMmuInterface
{
public:
    DmgMmu() = delete;
    DmgMmu(std::unique_ptr<GbApu> apu,
        std::shared_ptr<GbCartridgeInterface> cartridge,
        std::unique_ptr<GbGpuInterface> gpu,
        std::unique_ptr<GbHighRam> highRam,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::unique_ptr<GbJoypad> joypad,
        std::unique_ptr<DmgWorkRam> workRam,
        std::unique_ptr<GbTimer> timer);
    ~DmgMmu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
    bool isCpuStalled() const override;
    void triggerOamBugWrite(uint16_t address, int rowOffset = 0) override;
    void triggerOamBugRead(uint16_t address) override;
    void triggerOamBugReadIncrease(uint16_t address) override;

    /// Boot ROM mapping.  The image uses the standard CGB layout, so the
    /// 0x0100-0x01FF cartridge-header gap is never served from it.
    void setBootRom(std::span<const uint8_t> rom) { bootRom = rom; }
    void setBootRomActive(bool active) { bootRomActive = active; }
    bool isBootRomActive() const { return bootRomActive; }

protected:
    GbGpuInterface& gpu() const { return *gpu_; }
    DmgWorkRam& workRam() const { return *workRam_; }
    GbTimer& timer() const { return *timer_; }

private:
    std::unique_ptr<GbApu> apu;
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::unique_ptr<GbGpuInterface> gpu_;
    std::unique_ptr<GbHighRam> highRam;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    std::unique_ptr<GbJoypad> joypad;
    std::unique_ptr<DmgWorkRam> workRam_;
    std::unique_ptr<GbTimer> timer_;
    std::span<const uint8_t> bootRom;
    bool bootRomActive = false;
};

}  // namespace qnemu
