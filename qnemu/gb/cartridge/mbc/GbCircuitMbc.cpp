/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <cassert>

#include "qnemu/gb/cartridge/mbc/GbCircuitMbc.h"

namespace qnemu
{

GbCircuitMbc::GbCircuitMbc(std::vector<std::array<uint8_t, romBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, ramBankSize>>&& ramBanks, uint8_t type) :
    romBanks(romBanks),
    ramBanks(ramBanks),
    type(type)
{
}

bool GbCircuitMbc::accepts(uint16_t address) const
{
    return (address < 0x8000) || (address >= 0xA000 && address < 0xC000);
}

uint8_t GbCircuitMbc::read(uint16_t address) const
{
    if (address < 0x4000) {
        return romBanks.at(0).at(address);
    }
    if (address >= 0x4000 && address < 0x8000) {
        return romBanks.at(1).at(address - 0x4000);
    }
    if (address >= 0xA000 && address < 0xC000) {
        if (type != 0) {
            return ramBanks.at(0).at(address - 0xA000);
        }
        assert(false && "No Ram available");
        return 0xFF;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbCircuitMbc::write(uint16_t address, const uint8_t& value)
{
    if (address >= 0xA000 && address < 0xC000) {
        if (type != 0) {
            ramBanks.at(0).at(address - 0xA000) = value;
            return;
        }
        // assert(false && "No Ram available");
    }
    // assert(false && "Wrong address");
}

void GbCircuitMbc::step()
{
}

void GbCircuitMbc::reset()
{
}

}  // namespace qnemu