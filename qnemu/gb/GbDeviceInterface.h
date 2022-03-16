/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/DeviceInterface.h"

namespace qnemu
{

class GbDeviceInterface : public DeviceInterface<uint8_t, uint16_t>
{
public:
    static inline bool interruptMasterEnabled = false;
};

}  // namespace qnemu