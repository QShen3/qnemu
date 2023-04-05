/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbWorkRam : public GbDeviceInterface
{
public:
    GbWorkRam();
    ~GbWorkRam() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    struct {
        uint8_t workRamBank;
    } registers;
    std::array<std::array<uint8_t, 0x1000>, 8> data;
};

}  // namespace qnemu