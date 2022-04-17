/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/CpuInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbCpuInterface : public CpuInterface
{
public:
    virtual void jumpToAddress(uint16_t address) = 0;
    virtual void addDevice(std::shared_ptr<GbDeviceInterface> device) = 0;

    virtual uint8_t readByte(uint16_t address) const = 0;
    virtual void writeByte(uint16_t address, uint8_t value) = 0;
};

}  // namespace qnemu