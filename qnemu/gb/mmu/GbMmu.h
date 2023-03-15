/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/mmu/GbMmuInterface.h"

namespace qnemu
{

class GbMmu : public GbMmuInterface
{
public:
    GbMmu();
    ~GbMmu() = default;

    uint8_t read(uint16_t address) const;
    void write(uint16_t address, const uint8_t& value);
};

}  // namespace qnemu