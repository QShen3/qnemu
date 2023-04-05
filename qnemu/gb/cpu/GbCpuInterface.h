/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/CpuInterface.h"
#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/mmu/GbMmuInterface.h"

namespace qnemu
{

class GbCpuInterface : public CpuInterface
{
public:
    virtual bool isInHaltMode() const = 0;
    virtual void exitHaltMode() = 0;
    virtual bool isInStopMode() const = 0;
    virtual void exitStopMode() = 0;
    virtual void jumpToAddress(uint16_t address) = 0;
    virtual void cancelInterrupt() = 0;
    virtual void addDevice(std::shared_ptr<GbDeviceInterface> device) = 0;
    virtual void addMmu(std::unique_ptr<GbMmuInterface> mmu) = 0;
    virtual void addDisplay(std::shared_ptr<DisplayInterface> display) = 0;

    virtual uint8_t readByte(uint16_t address) const = 0;
    virtual void writeByte(uint16_t address, uint8_t value) = 0;
};

}  // namespace qnemu