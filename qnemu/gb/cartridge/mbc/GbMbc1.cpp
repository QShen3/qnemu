/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstring>

#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbMbc1::GbMbc1(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(romBanks),
    ramBanks(ramBanks),
    type(type)
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
        if ((registers.ramEnable & 0b1111) != ramEnableFlag) {
            assert(false && "Ram is disabled");
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
        if ((registers.ramEnable & 0b1111) != ramEnableFlag) {
            assert(false && "Ram is disabled");
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
            bankNumber = ((registers.ramBankNumber & 0b11) << 5);
        }
    } else {
        bankNumber = registers.romBankNumber & (romBanks.size() - 1);
        bankNumber = bankNumber == 0 ? 1 : bankNumber;
        if (romBanks.size() > 32) {
            bankNumber |= (registers.ramBankNumber & ((romBanks.size() >> 5) - 1)) << 5;
        }
    }

    return bankNumber;
}

uint8_t GbMbc1::getRamBankNumber() const
{
    uint8_t bankNumber = 0;

    if (registers.bankingModeSelect != 0) {
        if (ramBanks.size() == 0) {
            assert(false && "No Ram available");
        }
        bankNumber = registers.ramBankNumber & (ramBanks.size() - 1);
    }

    return bankNumber;
}

}  // namespace qnemu