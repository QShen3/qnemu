/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"

namespace qnemu
{

class GbChannel1 : public GbChannelInterface
{
public:
    GbChannel1() = default;
    ~GbChannel1() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t getData() const override;

private:
    uint8_t data;

};

}  // namespace qnemu