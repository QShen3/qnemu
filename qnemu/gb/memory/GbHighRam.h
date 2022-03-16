/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbHighRam : public GbDeviceInterface
{
public:
    GbHighRam() = default;
    ~GbHighRam() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void reset() override;
    void step() override;
    bool accepts(uint16_t address) const override;

private:
    std::array<uint8_t, 0x7F> data;
};

}  // namespace qnemu