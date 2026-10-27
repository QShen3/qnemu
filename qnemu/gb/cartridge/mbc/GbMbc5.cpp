/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbMbc5.h"

namespace qnemu
{

GbMbc5::GbMbc5(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(std::move(romBanks)),
    ramBanks(std::move(ramBanks)),
    type(type)
{
    GbMbc5::reset();
}

uint8_t GbMbc5::read(uint16_t address) const
{
    if (address <= MemoryRomBank01End) {
        if (address <= MemoryRomBank00End) {
            return romBanks.at(0).at(address);
        }

        const uint8_t bank = static_cast<uint8_t>(romBankNumber & (romBanks.size() - 1));
        return romBanks.at(bank).at(address - MemoryRomBank01Start);
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled || ramBanks.empty()) {
            return 0xFF;
        }
        const uint8_t bank = getRamBankNumber();
        return ramBanks.at(bank).at(address - externalRamStart);
    }

    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc5::write(uint16_t address, const uint8_t value)
{
    if (address < romBankLowAddress) {
        ramEnabled = (value & 0x0F) == 0x0A;
        return;
    }
    if (address < romBankHighAddress) {
        romBankNumber = static_cast<uint16_t>((romBankNumber & 0x100) | value);
        return;
    }
    if (address < ramBankNumberAddress) {
        romBankNumber = static_cast<uint16_t>((romBankNumber & 0x0FF)
            | ((value & 0x01) << 8));
        return;
    }
    if (address <= MemoryRomBank01End) {
        ramBankNumber = static_cast<uint8_t>(value & (isRumble() ? 0x07 : 0x0F));
        return;
    }

    if (address >= externalRamStart && address <= externalRamEnd) {
        if (!ramEnabled || ramBanks.empty()) {
            return;
        }
        const uint8_t bank = getRamBankNumber();
        ramBanks.at(bank).at(address - externalRamStart) = value;
        return;
    }

    assert(false && "Wrong address");
}

void GbMbc5::reset()
{
    ramEnabled = false;
    romBankNumber = 1;  // MBC5 powers up with bank 1 at 4000-7FFF
    ramBankNumber = 0;
}

bool GbMbc5::isRumble() const
{
    return type >= 0x1C && type <= 0x1E;
}

uint8_t GbMbc5::getRamBankNumber() const
{
    if (ramBanks.empty()) {
        return 0;
    }
    return static_cast<uint8_t>(ramBankNumber & (ramBanks.size() - 1));
}

}  // namespace qnemu
