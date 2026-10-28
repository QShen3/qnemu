/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbMbc2.h"

namespace qnemu
{

GbMbc2::GbMbc2(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(std::move(romBanks)),
    type(type)
{
    (void)ramBanks;  // MBC2 has its own 512x4-bit RAM
    GbMbc2::reset();
}

uint8_t GbMbc2::read(uint16_t address) const
{
    if (address <= MemoryRomBank01End) {
        if (address <= MemoryRomBank00End) {
            return romBanks.at(0).at(address);
        }

        uint8_t bank = static_cast<uint8_t>(romBankNumber & 0x0F);
        if (bank == 0) {
            bank = 1;
        }
        bank = static_cast<uint8_t>(bank & (romBanks.size() - 1));
        return romBanks.at(bank).at(address - MemoryRomBank01Start);
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled) {
            return 0xFF;
        }
        const uint16_t index = static_cast<uint16_t>((address - externalRamStart) & 0x1FF);
        return static_cast<uint8_t>(0xF0 | (ram.at(index) & 0x0F));
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc2::write(uint16_t address, const uint8_t value)
{
    if (address <= MemoryRomBank00End) {
        if ((address & 0x0100) == 0) {
            ramEnabled = (value & 0x0F) == 0x0A;
        } else {
            romBankNumber = static_cast<uint8_t>(value & 0x0F);
            if (romBankNumber == 0) {
                romBankNumber = 1;
            }
        }
        return;
    }

    if (address <= MemoryRomBank01End) {
        // MBC2 has no registers at 4000-7FFF.
        return;
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled) {
            return;
        }
        const uint16_t index = static_cast<uint16_t>((address - externalRamStart) & 0x1FF);
        ram.at(index) = static_cast<uint8_t>(value & 0x0F);
        return;
    }

    // Writes outside the cartridge's control and RAM ranges are ignored on
    // real hardware (the bus still sees them, but the MBC does not react).
    assert(false && "Wrong address");
}

void GbMbc2::reset()
{
    ramEnabled = false;
    romBankNumber = 1;
}

bool GbMbc2::hasBattery() const
{
    return type == 0x06;
}

std::size_t GbMbc2::ramSize() const
{
    return ram.size();
}

std::vector<uint8_t> GbMbc2::dumpRam() const
{
    std::vector<uint8_t> data(ram.size(), 0);
    for (std::size_t i = 0; i < ram.size(); ++i) {
        data.at(i) = static_cast<uint8_t>(ram.at(i) & 0x0F);
    }
    return data;
}

void GbMbc2::loadRam(const std::vector<uint8_t>& data)
{
    for (std::size_t i = 0; i < ram.size(); ++i) {
        ram.at(i) = i < data.size() ? static_cast<uint8_t>(data.at(i) & 0x0F) : 0;
    }
}

}  // namespace qnemu
