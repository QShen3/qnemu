/* 
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbChannelInterface : public GbDeviceInterface
{
public:
    virtual ~GbChannelInterface() override = default;

    virtual uint8_t getData() const = 0;
};

}  // namespace qnemu