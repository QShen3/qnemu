/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbGpuInterface : public GbDeviceInterface
{
public:
    virtual ~GbGpuInterface() override = default;

    virtual uint8_t currentMode() const = 0;
    virtual bool isLcdEnable() const = 0;
};

}  // namespace qnemu
