/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemu
{

class GbCircuitMbc : public GbMbcInterface
{
public:
    GbCircuitMbc() = delete;
    GbCircuitMbc(std::vector<std::array<uint8_t, romBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, ramBankSize>>&& ramBanks, uint8_t type);
    ~GbCircuitMbc() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void reset() override;
    void step() override;
    bool accepts(uint16_t address) const override;

private:
    std::vector<std::array<uint8_t, romBankSize>> romBanks;
    std::vector<std::array<uint8_t, ramBankSize>> ramBanks;
    uint8_t type;
};

}  // namespace qnemu