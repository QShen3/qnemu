/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbOam.h"

namespace qnemu
{

GbOam::GbOam(const GbCartridgeInterface& cartridge,
        const GbDeviceInterface& videoRam,
        const GbDeviceInterface& workRam) :
    cartridge(cartridge),
    videoRam(videoRam),
    workRam(workRam)
{
    GbOam::reset();
}

uint8_t GbOam::read(uint16_t address) const
{
    if (address >= OamStart && address <= OamEnd) {
        return data.at(address - OamStart);
    }
    if (address == 0xFF46) {
        return registers.dmaTransferAndStartAddress;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbOam::write(uint16_t address, const uint8_t& value)
{
    if (address >= OamStart && address <= OamEnd) {
        data.at(address - OamStart) = value;
    }
    if (address == 0xFF46) {
        registers.dmaTransferAndStartAddress = value;
        if (!isDmaInProgress) {
            isDmaInProgress = true;
            dmaTicks = 640;
        }
    }
}

void GbOam::step()
{
    if (!isDmaInProgress) {
        return;
    }

    dmaTicks--;

    if (dmaTicks > 0) {
        return;
    }
    isDmaInProgress = false;
    for (uint8_t i = 0; i < 0xA0; i++) {
        uint16_t address = registers.dmaTransferAndStartAddress * 0x100 + i;
        if (address <= MemoryRomBank01End) {
            data.at(i) = cartridge.read(address);
        } else if (address >= VideoRamStart && address <= VideoRamEnd) {
            data.at(i) = videoRam.read(address);
        } else if (address >= ExternalRamStart && address <= ExternalRamEnd) {
            data.at(i) = cartridge.read(address);
        } else if (address >= WorkRamBank00Start && address <= WorkRamBank01End) {
            data.at(i) = workRam.read(address);
        } else {
            assert(false && "Wrong oam address");
        }
    }
}

void GbOam::reset()
{
    std::memset(&registers, 0, sizeof(registers));

    isDmaInProgress = false;
    dmaTicks = 0;
}

uint8_t GbOam::at(uint16_t address) const
{
    return data.at(address);
}

}  // namespace qnemu