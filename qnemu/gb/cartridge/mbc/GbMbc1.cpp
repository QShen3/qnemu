/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstring>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbMbc1::GbMbc1(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type,
        bool mbc1m) :
    romBanks(std::move(romBanks)),
    ramBanks(std::move(ramBanks)),
    type(type),
    mbc1m(mbc1m)
{
    GbMbc1::reset();
}

uint8_t GbMbc1::read(uint16_t address) const
{
    if (address <= MemoryRomBank01End) {
        const uint8_t bankNumber = getRomBankNumberForAddress(address);
        if (address <= MemoryRomBank00End) {
            return romBanks.at(bankNumber).at(address);
        }
        return romBanks.at(bankNumber).at(address - MemoryRomBank01Start);
    }
    if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        if ((registers.ramEnable & 0b1111) != ramEnableFlag || ramBanks.empty()) {
            return 0xFF;
        }
        const uint8_t bankNumber = getRamBankNumber();
        return ramBanks.at(bankNumber).at(address - ExternalRamStart);
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc1::write(uint16_t address, const uint8_t value)
{
    if (address < romBankNumberAddress) {
        registers.ramEnable = value;
        return;
    }
    if (address < ramBankNumberAddress) {
        registers.romBankNumber = value & 0b11111;
        return;
    }
    if (address < bankingModeSelectAddress) {
        registers.ramBankNumber = value & 0b11;
        return;
    }
    if (address <= MemoryRomBank01End) {
        registers.bankingModeSelect = value & 0b1;
        return;
    }
    if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        if ((registers.ramEnable & 0b1111) != ramEnableFlag || ramBanks.empty()) {
            return;
        }
        const uint8_t bankNumber = getRamBankNumber();
        ramBanks.at(bankNumber).at(address - ExternalRamStart) = value;
        return;
    }
    assert(false && "Wrong address");
}

void GbMbc1::reset()
{
    (void)type;
    std::memset(&registers, 0, sizeof(registers));
}

uint8_t GbMbc1::getRomBankNumberForAddress(uint16_t address) const
{
    uint8_t bankNumber = 0;

    if (address < 0x4000) {
        if (registers.bankingModeSelect != 0) {
            // MBC1M wires the secondary register to bits 4-5 instead of 5-6.
            const uint8_t shift = mbc1m ? 4 : 5;
            bankNumber = static_cast<uint8_t>(((registers.ramBankNumber & 0b11) << shift)
                & (romBanks.size() - 1));
        }
    } else {
        // The 00 -> 01 translation uses the full 5-bit register, before the
        // bank number is masked to the ROM size.
        uint8_t rawBank = static_cast<uint8_t>(registers.romBankNumber & 0b11111);
        if (rawBank == 0) {
            rawBank = 1;
        }

        if (mbc1m) {
            // MBC1M ignores the top bit of the main banking register and
            // uses the secondary register for bits 4-5.
            bankNumber = static_cast<uint8_t>((rawBank & 0x0F)
                | ((registers.ramBankNumber & 0b11) << 4));
            bankNumber = static_cast<uint8_t>(bankNumber & (romBanks.size() - 1));
        } else {
            bankNumber = static_cast<uint8_t>(rawBank & (romBanks.size() - 1));
            if (romBanks.size() > 32) {
                bankNumber |= static_cast<uint8_t>(
                    (registers.ramBankNumber & ((romBanks.size() >> 5) - 1)) << 5);
            }
        }
    }

    return bankNumber;
}

uint8_t GbMbc1::getRamBankNumber() const
{
    uint8_t bankNumber = 0;

    if (registers.bankingModeSelect != 0 && !ramBanks.empty()) {
        bankNumber = static_cast<uint8_t>(registers.ramBankNumber & (ramBanks.size() - 1));
    }

    return bankNumber;
}

}  // namespace qnemu