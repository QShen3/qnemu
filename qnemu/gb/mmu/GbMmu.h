/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

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
    GbMmu(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<GbGpuInterface> gpu,
        std::shared_ptr<GbHighRam> highRam,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::shared_ptr<GbJoypad> joypad,
        std::shared_ptr<GbWorkRam> workRam,
        std::shared_ptr<GbTimer> timer);
    ~GbMmu() = default;

    uint8_t read(uint16_t address) const;
    void write(uint16_t address, const uint8_t& value);

private:
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::shared_ptr<GbGpuInterface> gpu;
    std::shared_ptr<GbHighRam> highRam;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    std::shared_ptr<GbJoypad> joypad;
    std::shared_ptr<GbWorkRam> workRam;
    std::shared_ptr<GbTimer> timer;
};

}  // namespace qnemu