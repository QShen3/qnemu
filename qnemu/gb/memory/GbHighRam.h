/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

/// FF80-FFFE High RAM.  It is not cleared by reset, matching hardware.
class GbHighRam : public GbDeviceInterface
{
public:
    GbHighRam() = default;
    ~GbHighRam() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    std::array<uint8_t, 0x7F> data{};
};

}  // namespace qnemu
