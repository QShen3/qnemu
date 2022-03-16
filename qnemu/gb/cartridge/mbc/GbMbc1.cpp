/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstring>

#include "qnemu/gb/cartridge/mbc/GbMbc1.h"

namespace qnemu
{

GbMbc1::GbMbc1(std::vector<std::array<uint8_t, romBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, ramBankSize>>&& ramBanks, uint8_t type) :
    romBanks(romBanks),
    ramBanks(ramBanks),
    type(type)
{
    GbMbc1::reset();
}

uint8_t GbMbc1::read(uint16_t address) const
{
    if (address < 0x8000) {
        uint8_t bankNumber = getRomBankNumberForAddress(address);
        if (address < 0x4000) {
            return romBanks.at(bankNumber).at(address);
        }
        return romBanks.at(bankNumber).at(address - 0x4000);
    }
    if (address >= 0xA000 && address < 0xC000) {
        if ((registers.ramEnable & 0b1111) != ramEnableFlag) {
            assert(false && "Ram is disabled");
            return 0xFF;
        }
        uint8_t bankNumber = getRamBankNumber();
        return ramBanks.at(bankNumber).at(address - 0xA000);
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbMbc1::write(uint16_t address, const uint8_t& value)
{
    if (address < romBankNumberAddress) {
        registers.ramEnable = value;
    } else if (address >= romBankNumberAddress && address < ramBankNumberAddress) {
        registers.romBankNumber = value & 0b11111;
    } else if (address >= ramBankNumberAddress && address < bankingModeSelectAddress) {
        registers.ramBankNumber = value & 0b11;
    } else if (address >= bankingModeSelectAddress && address < 0x8000) {
        registers.bankingModeSelect = value & 0b1;
    } else if (address >= 0xA000 && address < 0xC000) {
        if ((registers.ramEnable & 0b1111) != ramEnableFlag) {
            assert(false && "Ram is disabled");
            return;
        }
        uint8_t bankNumber = getRamBankNumber();
        ramBanks.at(bankNumber).at(address - 0xA000) = value;
    }
    assert(false && "Wrong address");
}

void GbMbc1::reset()
{
    std::memset(&registers, 0, sizeof(registers));
}

void GbMbc1::step()
{
}

bool GbMbc1::accepts(uint16_t address) const
{
    return (address < 0x8000) || (address >= 0xA000 && address < 0xC000);
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