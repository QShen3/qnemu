/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cassert>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbVideoRam.h"

namespace qnemu
{

GbVideoRam::GbVideoRam(const GbCartridgeInterface& cartridge, std::shared_ptr<GbCpuInterface> cpu) : cartridge(cartridge), cpu(cpu)
{
    GbVideoRam::reset();
}

bool GbVideoRam::accepts(uint16_t address) const
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        return true;
    }
    if (address == 0xFF4F) {
        return true;
    }
    if (address >= 0xFF51 && address <= 0xFF55) {
        return true;
    }
    return false;
}

uint8_t GbVideoRam::read(uint16_t address) const
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        return videoRamBanks.at(cartridge.isGbcCartridge() ? registers.videoRamBank : 0).at(address - VideoRamStart);
    }
    if (address == 0xFF4F) {
        return registers.videoRamBank;
    }
    if (address == 0xFF51) {
        return registers.newDMASourceHigh;
    }
    if (address == 0xFF52) {
        return registers.newDMASourceLow;
    }
    if (address == 0xFF53) {
        return registers.newDMADestinationHigh;
    }
    if (address == 0xFF54) {
        return registers.newDMADestinationLow;
    }
    if (address == 0xFF55) {
        return registers.newDMALength;
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbVideoRam::write(uint16_t address, const uint8_t& value)
{
    if (address >= VideoRamStart && address <= VideoRamEnd) {
        videoRamBanks.at(cartridge.isGbcCartridge() ? registers.videoRamBank : 0).at(address - VideoRamStart) = value;
    }
    if (address == 0xFF4F) {
        registers.videoRamBank = value;
    }
    if (address == 0xFF51) {
        registers.newDMASourceHigh = value;
    }
    if (address == 0xFF52) {
        registers.newDMASourceLow = (value & 0b11110000);
    }
    if (address == 0xFF53) {
        registers.newDMADestinationHigh = (value & 0b00011111);
    }
    if (address == 0xFF54) {
        registers.newDMADestinationLow = (value & 0b11110000);
    }
    if (address == 0xFF55) {
        if (isDmaInProgress && ((value & 0b10000000) == 0)) {
            isDmaInProgress = false;
            registers.newDMALength = registers.newDMALength | 0b10000000;
        }
        if (!isDmaInProgress) {
            isDmaInProgress = true;
            registers.newDMALength = value & 0b01111111;
            source = (static_cast<uint16_t>(registers.newDMASourceHigh) << 8) | registers.newDMASourceLow;
            destination = (static_cast<uint16_t>(registers.newDMADestinationHigh) << 8) | registers.newDMADestinationLow;
            length = (registers.newDMALength + 1) * 0x10;
            isHBlankDma = ((value & 0b10000000) > 0);
            dmaTicks = 0x20;
        }
    }
}

void GbVideoRam::step()
{
    if (!isDmaInProgress) {
        return;
    }

    if (isHBlankDma && (cpu.lock()->gpuMode() != 0)) {
        return;
    }

    dmaTicks--;
    if (dmaTicks > 0) {
        return;
    }

    for (uint8_t i = 0; i < 0x10; i++) {
        write(destination, cpu.lock()->readByte(source));
    }
    length = length - 0x10;
    if (length == 0) {
        isDmaInProgress = false;
        registers.newDMALength = 0xFF;
        return;
    }
    registers.newDMALength = registers.newDMALength - 1;
    source = source + 0x10;
    destination = destination + 0x10;
    dmaTicks = 0x20;
}

void GbVideoRam::reset()
{
    registers.videoRamBank = 0xFF;
    registers.newDMASourceHigh = 0xFF;
    registers.newDMASourceLow = 0xFF;
    registers.newDMADestinationHigh = 0xFF;
    registers.newDMADestinationLow = 0xFF;
    registers.newDMALength = 0xFF;

    dmaTicks = 0;
    isHBlankDma = false;
    isDmaInProgress = false;

    source = 0;
    destination = 0;
    length = 0;
}

const std::array<uint8_t, VideoRamBankSize>& GbVideoRam::getBank(uint8_t index) const
{
    return videoRamBanks.at(index);
}

}  // namespace qnemu
