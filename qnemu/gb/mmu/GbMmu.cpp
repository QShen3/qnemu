/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/mmu/GbMmu.h"

namespace qnemu
{

GbMmu::GbMmu(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<GbGpuInterface> gpu) :
    cartridge(cartridge),
    gpu(gpu)
{

}

uint8_t GbMmu::read(uint16_t address) const
{
    if (address >= MemoryRomBank00Start && address <= MemoryRomBank01End) {
        return cartridge->read(address);
    } else if (address >= VideoRamStart && address <= VideoRamEnd) {
        return gpu->read(address);
    } else if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        return cartridge->read(address);
    } else {
        return 0xFF;
    }
}

void GbMmu::write(uint16_t address, const uint8_t& value)
{
    if (address >= MemoryRomBank00Start && address <= MemoryRomBank01End) {
        cartridge->write(address, value);
    } else if (address >= VideoRamStart && address <= VideoRamEnd) {
        gpu->write(address, value);
    } else if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        cartridge->write(address, value);
    } else {
        return;
    }
}

}  // namespace qnemu