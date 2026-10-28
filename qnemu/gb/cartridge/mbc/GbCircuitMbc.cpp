/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbCircuitMbc.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbCircuitMbc::GbCircuitMbc(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    romBanks(std::move(romBanks)),
    ramBanks(std::move(ramBanks)),
    type(type)
{
}

uint8_t GbCircuitMbc::read(uint16_t address) const
{
    if (address <= MemoryRomBank00End) {
        return romBanks.at(0).at(address);
    }
    if (address <= MemoryRomBank01End) {
        return romBanks.at(1).at(address - MemoryRomBank01Start);
    }
    if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        if (type != 0 && !ramBanks.empty()) {
            return ramBanks.at(0).at(address - ExternalRamStart);
        }
        return 0xFF;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbCircuitMbc::write(uint16_t address, const uint8_t value)
{
    if (address >= ExternalRamStart && address <= ExternalRamEnd) {
        if (type != 0 && !ramBanks.empty()) {
            ramBanks.at(0).at(address - ExternalRamStart) = value;
        }
        return;
    }
    // assert(false && "Wrong address");
}

void GbCircuitMbc::reset()
{
}

bool GbCircuitMbc::hasBattery() const
{
    return type == 0x09;
}

std::size_t GbCircuitMbc::ramSize() const
{
    return ramBanks.size() * RamBankSize;
}

std::vector<uint8_t> GbCircuitMbc::dumpRam() const
{
    std::vector<uint8_t> data;
    data.reserve(ramSize());
    for (const auto& bank : ramBanks) {
        data.insert(data.end(), bank.begin(), bank.end());
    }
    return data;
}

void GbCircuitMbc::loadRam(const std::vector<uint8_t>& data)
{
    std::size_t offset = 0;
    for (auto& bank : ramBanks) {
        for (auto& byte : bank) {
            byte = offset < data.size() ? data.at(offset++) : 0xFF;
        }
    }
}

}  // namespace qnemu