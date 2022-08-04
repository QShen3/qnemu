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

class GbMbc1 : public GbMbcInterface
{
public:
    static constexpr uint16_t ramEnableAddress = 0;
    static constexpr uint16_t romBankNumberAddress = 0x2000;
    static constexpr uint16_t ramBankNumberAddress = 0x4000;
    static constexpr uint16_t bankingModeSelectAddress = 0x6000;

    GbMbc1() = delete;
    GbMbc1(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc1() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    static constexpr uint8_t ramEnableFlag = 0xA;

    uint8_t getRomBankNumberForAddress(uint16_t address) const;
    uint8_t getRamBankNumber() const;

    struct Registers
    {
        uint8_t ramEnable;
        uint8_t romBankNumber;
        uint8_t ramBankNumber;
        uint8_t bankingModeSelect;
    } registers;

    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, RamBankSize>> ramBanks;
    uint8_t type;
};

}  // namespace qnemu