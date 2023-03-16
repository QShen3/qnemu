/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbMmuInterface.h"

namespace qnemu
{

class GbMmu : public GbMmuInterface
{
public:
    GbMmu() = delete;
    GbMmu(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<GbGpuInterface> gpu,
        std::shared_ptr<GbWorkRam> workRam);
    ~GbMmu() = default;

    uint8_t read(uint16_t address) const;
    void write(uint16_t address, const uint8_t& value);

private:
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::shared_ptr<GbGpuInterface> gpu;
    std::shared_ptr<GbWorkRam> workRam;
};

}  // namespace qnemu