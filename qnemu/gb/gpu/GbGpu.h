/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbGpu : GbDeviceInterface
{
public:
    GbGpu() = default;
    ~GbGpu() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
};

}  // namespace qnemu