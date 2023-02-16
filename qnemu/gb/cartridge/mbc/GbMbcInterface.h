/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

class GbMbcInterface
{
public:
    virtual ~GbMbcInterface() = default;

    virtual uint8_t read(uint16_t address) const = 0;
    virtual void write(uint16_t address, const uint8_t value) = 0;
    virtual void reset() = 0;
};

}  // namespace qnemu