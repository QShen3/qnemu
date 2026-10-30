/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbGpuInterface : public GbDeviceInterface
{
public:
    virtual ~GbGpuInterface() override = default;

    virtual uint8_t currentMode() const = 0;
    virtual bool isLcdEnable() const = 0;

    /// DMG OAM corruption hooks; no-op on GPUs without the quirk.
    virtual void triggerOamBugWrite(uint16_t /*address*/) {}
    virtual void triggerOamBugRead(uint16_t /*address*/) {}
    virtual void triggerOamBugReadIncrease(uint16_t /*address*/) {}
};

}  // namespace qnemu
