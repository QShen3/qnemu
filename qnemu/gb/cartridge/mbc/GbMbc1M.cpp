/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbc1M.h"

namespace qnemu
{

GbMbc1M::GbMbc1M(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    GbMbc1(std::move(romBanks), std::move(ramBanks), type)
{
}

uint8_t GbMbc1M::getRomBankNumberForAddress(uint16_t address) const
{
    uint8_t bankNumber = 0;

    if (address < 0x4000) {
        if (registers.bankingModeSelect != 0) {
            bankNumber = static_cast<uint8_t>(((registers.ramBankNumber & 0b11) << 4)
                & (romBanks.size() - 1));
        }
    } else {
        // The full 5-bit register is used for the 00 -> 01 translation, but
        // only its low four bits participate in banking.
        uint8_t rawBank = static_cast<uint8_t>(registers.romBankNumber & 0b11111);
        if (rawBank == 0) {
            rawBank = 1;
        }
        bankNumber = static_cast<uint8_t>((rawBank & 0x0F)
            | ((registers.ramBankNumber & 0b11) << 4));
        bankNumber = static_cast<uint8_t>(bankNumber & (romBanks.size() - 1));
    }

    return bankNumber;
}

uint8_t GbMbc1M::getRamBankNumber() const
{
    // The secondary register is wired to the ROM address lines on MBC1M.
    return 0;
}

}  // namespace qnemu
