/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbc30.h"

namespace qnemu
{

GbMbc30::GbMbc30(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) :
    GbMbc3(std::move(romBanks), std::move(ramBanks), type)
{
}

}  // namespace qnemu
