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
 * MBC5: up to 8 MiB ROM and 128 KiB RAM.  Unlike MBC1/MBC3, bank 0 is a
 * valid bank at 4000-7FFF.  Rumble variants use bit 3 of the RAM bank
 * register for the motor instead of the RAM bank number.
 */
class GbMbc5 : public GbMbcInterface
{
public:
    static constexpr uint16_t ramEnableAddress = 0x0000;
    static constexpr uint16_t romBankLowAddress = 0x2000;
    static constexpr uint16_t romBankHighAddress = 0x3000;
    static constexpr uint16_t ramBankNumberAddress = 0x4000;
    static constexpr uint16_t externalRamStart = 0xA000;
    static constexpr uint16_t externalRamEnd = 0xBFFF;

    GbMbc5() = delete;
    GbMbc5(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc5() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t value) override;
    void reset() override;

    bool hasBattery() const override;
    std::size_t ramSize() const override;
    std::vector<uint8_t> dumpRam() const override;
    void loadRam(const std::vector<uint8_t>& data) override;

private:
    bool isRumble() const;
    uint8_t getRamBankNumber() const;

    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, RamBankSize>> ramBanks;
    uint8_t type;

    bool ramEnabled = false;
    uint16_t romBankNumber = 0;
    uint8_t ramBankNumber = 0;
};

}  // namespace qnemu
