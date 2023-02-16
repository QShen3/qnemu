/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <cassert>

#include "qnemu/gb/cartridge/mbc/GbCircuitMbc.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbCircuitMbc::GbCircuitMbc(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(romBanks),
    ramBanks(ramBanks),
    type(type)
{
}

uint8_t GbCircuitMbc::read(uint16_t address) const
{
    if (address <= MemoryRomBank00End) {
        return romBanks.at(0).at(address);
    }
    if (address >= MemoryRomBank01Start && address <= MemoryRomBank01End) {
        return romBanks.at(1).at(address - MemoryRomBank01Start);
    }
    if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        if (type != 0) {
            return ramBanks.at(0).at(address - ExternalRamStart);
        }
        assert(false && "No Ram available");
        return 0xFF;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbCircuitMbc::write(uint16_t address, const uint8_t value)
{
    if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        if (type != 0) {
            ramBanks.at(0).at(address - ExternalRamStart) = value;
            return;
        }
        // assert(false && "No Ram available");
    }
    // assert(false && "Wrong address");
}

void GbCircuitMbc::reset()
{
}

}  // namespace qnemu