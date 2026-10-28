/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbHuc1.h"

namespace qnemu
{

GbHuc1::GbHuc1(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(std::move(romBanks)),
    ramBanks(std::move(ramBanks)),
    type(type)
{
    GbHuc1::reset();
}

uint8_t GbHuc1::read(uint16_t address) const
{
    if (address <= MemoryRomBank01End) {
        if (address <= MemoryRomBank00End) {
            return romBanks.at(0).at(address);
        }

        const uint8_t bank = static_cast<uint8_t>(romBankNumber & (romBanks.size() - 1));
        return romBanks.at(bank).at(address - MemoryRomBank01Start);
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (irMode) {
            // No IR peer is connected.  C0 means "no light seen"; the local
            // output latch is kept for a future IR backend.
            return 0xC0;
        }
        if (ramBanks.empty()) {
            return 0xFF;
        }
        const uint8_t bank = static_cast<uint8_t>(ramBankNumber & (ramBanks.size() - 1));
        return ramBanks.at(bank).at(address - externalRamStart);
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbHuc1::write(uint16_t address, const uint8_t value)
{
    if (address < romBankNumberAddress) {
        irMode = (value & 0x0F) == 0x0E;
        if (!irMode) {
            irOutput = 0;
        }
        return;
    }
    if (address < ramBankNumberAddress) {
        romBankNumber = value;
        return;
    }
    if (address < 0x6000) {
        ramBankNumber = value;
        return;
    }
    if (address <= MemoryRomBank01End) {
        // 6000-7FFF is not connected on HuC1.
        return;
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (irMode) {
            irOutput = static_cast<uint8_t>(value & 0x01);
            return;
        }
        if (ramBanks.empty()) {
            return;
        }
        const uint8_t bank = static_cast<uint8_t>(ramBankNumber & (ramBanks.size() - 1));
        ramBanks.at(bank).at(address - externalRamStart) = value;
        return;
    }

    assert(false && "Wrong address");
}

void GbHuc1::reset()
{
    irMode = false;
    irOutput = 0;
    romBankNumber = 0;
    ramBankNumber = 0;
}

bool GbHuc1::hasBattery() const
{
    return type == 0xFF;
}

std::size_t GbHuc1::ramSize() const
{
    return ramBanks.size() * RamBankSize;
}

std::vector<uint8_t> GbHuc1::dumpRam() const
{
    std::vector<uint8_t> data;
    data.reserve(ramSize());
    for (const auto& bank : ramBanks) {
        data.insert(data.end(), bank.begin(), bank.end());
    }
    return data;
}

void GbHuc1::loadRam(const std::vector<uint8_t>& data)
{
    std::size_t offset = 0;
    for (auto& bank : ramBanks) {
        for (auto& byte : bank) {
            byte = offset < data.size() ? data.at(offset++) : 0xFF;
        }
    }
}

}  // namespace qnemu
