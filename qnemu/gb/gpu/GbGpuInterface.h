/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbGpuInterface : public GbDeviceInterface
{
public:
    virtual ~GbGpuInterface() = default;

    static constexpr size_t videoRamBankSize = 0x2000;
    static constexpr size_t spriteAttributeTableSize = 0xA0;
};

}  // namespace qnemu
