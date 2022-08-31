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

GbVideoRam::GbVideoRam(const GbCartridgeInterface& cartridge) : cartridge(cartridge)
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
        registers.newDMASourceLow = value;
    }
    if (address == 0xFF53) {
        registers.newDMADestinationHigh = value;
    }
    if (address == 0xFF54) {
        registers.newDMADestinationLow = value;
    }
    if (address == 0xFF55) {
        registers.newDMALength = value;
    }
}

void GbVideoRam::step()
{

}

void GbVideoRam::reset()
{
    registers.videoRamBank = 0xFF;
    registers.newDMASourceHigh = 0xFF;
    registers.newDMASourceLow = 0xFF;
    registers.newDMADestinationHigh = 0xFF;
    registers.newDMADestinationLow = 0xFF;
    registers.newDMALength = 0xFF;
}

const std::array<uint8_t, VideoRamBankSize>& GbVideoRam::getBank(uint8_t index) const
{
    return videoRamBanks.at(index);
}

}  // namespace qnemu
