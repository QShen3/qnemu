/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * MBC1M is an MBC1 variant used by 1 MiB multi-game compilations.
 *
 * It shares MBC1's registers, RAM and battery behaviour; only the bank
 * wiring differs:
 *   - the main register's top bit is ignored for banking (but still used for
 *     the 00 -> 01 translation)
 *   - the secondary register supplies ROM bank bits 4-5 instead of 5-6
 *   - the secondary register does not bank cartridge RAM
 */
class GbMbc1M : public GbMbc1
{
public:
    GbMbc1M() = delete;
    GbMbc1M(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc1M() override = default;

protected:
    uint8_t getRomBankNumberForAddress(uint16_t address) const override;
    uint8_t getRamBankNumber() const override;
};

}  // namespace qnemu
