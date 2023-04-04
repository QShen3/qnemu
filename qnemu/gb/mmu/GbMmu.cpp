/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbMmu.h"

namespace qnemu
{

GbMmu::GbMmu(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<GbGpuInterface> gpu,
        std::shared_ptr<GbJoypad> joypad,
        std::shared_ptr<GbWorkRam> workRam) :
    cartridge(cartridge),
    gpu(gpu),
    joypad(joypad),
    workRam(workRam)
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
    } else if (address >= WorkRamBank00Start && address <= EchoRamEnd) {
        return workRam->read(address);
    } else if (address >= OamStart && address <= OamEnd) {
        return gpu->read(address);
    } else if (address == 0xFF00) {
        return joypad->read(address);
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
    } else if (address >= WorkRamBank00Start && address <= EchoRamEnd) {
        workRam->write(address, value);
    } else if (address >= OamStart && address <= OamEnd) {
        gpu->write(address, value);
    } else if (address == 0xFF00) {
        joypad->write(address, value);
    } else {
        return;
    }
}

}  // namespace qnemu