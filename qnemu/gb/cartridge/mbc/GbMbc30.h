/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbc3.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * MBC30 is an MBC3 variant with a 4 MiB ROM and 64 KiB RAM.
 *
 * It behaves like MBC3 but uses the full 8-bit ROM bank register and all
 * eight 8 KiB RAM banks.  It is identified by a MBC3 header combined with a
 * 64 KiB RAM size (and/or 256 ROM banks).
 */
class GbMbc30 : public GbMbc3
{
public:
    GbMbc30() = delete;
    GbMbc30(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc30() override = default;

protected:
    uint8_t romBankMask() const override { return 0xFF; }
};

}  // namespace qnemu
