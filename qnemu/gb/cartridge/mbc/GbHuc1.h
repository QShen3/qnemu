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
 * HuC1: Hudson Soft's ROM/RAM banking mapper with an infrared port.
 *
 * RAM is always enabled in RAM mode; the 0000-1FFF register only switches
 * A000-BFFF between cartridge RAM and the IR register.  The IR hardware is
 * not connected to another Game Boy, so reads return "no light" (0xC0) and
 * writes only update the local output latch.
 */
class GbHuc1 : public GbMbcInterface
{
public:
    static constexpr uint16_t irSelectAddress = 0x0000;
    static constexpr uint16_t romBankNumberAddress = 0x2000;
    static constexpr uint16_t ramBankNumberAddress = 0x4000;
    static constexpr uint16_t externalRamStart = 0xA000;
    static constexpr uint16_t externalRamEnd = 0xBFFF;

    GbHuc1() = delete;
    GbHuc1(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbHuc1() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t value) override;
    void reset() override;

    bool hasBattery() const override;
    std::size_t ramSize() const override;
    std::vector<uint8_t> dumpRam() const override;
    void loadRam(const std::vector<uint8_t>& data) override;

private:
    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, RamBankSize>> ramBanks;
    uint8_t type;

    bool irMode = false;
    uint8_t irOutput = 0;
    uint8_t romBankNumber = 0;
    uint8_t ramBankNumber = 0;
};

}  // namespace qnemu
