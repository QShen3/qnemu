/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

class GbCircuitMbc : public GbMbcInterface
{
public:
    GbCircuitMbc() = delete;
    GbCircuitMbc(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbCircuitMbc() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, RamBankSize>> ramBanks;
    uint8_t type;
};

}  // namespace qnemu