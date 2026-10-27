/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * MBC2: up to 256 KiB ROM and 512 x 4 bits of built-in RAM.
 *
 * A8 of the address distinguishes RAM enable (A8=0) from the ROM bank
 * register (A8=1).  The built-in RAM occupies A000-A1FF and is mirrored
 * through the rest of A000-BFFF; only the low nibble is stored.
 */
class GbMbc2 : public GbMbcInterface
{
public:
    static constexpr uint16_t ramEnableOrRomBankAddress = 0x0000;
    static constexpr uint16_t externalRamStart = 0xA000;
    static constexpr uint16_t externalRamEnd = 0xBFFF;

    GbMbc2() = delete;
    GbMbc2(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc2() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t value) override;
    void reset() override;

private:
    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::array<uint8_t, 512> ram{};
    uint8_t type;
    bool ramEnabled = false;
    uint8_t romBankNumber = 1;
};

}  // namespace qnemu
