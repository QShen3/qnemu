/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/MmuInterface.h"

namespace qnemu
{

class GbMmuInterface : public MmuInterface<uint8_t, uint16_t>
{
public:
    /// CGB speed switch notification.  Peripherals that are affected by the
    /// double-speed clock (currently the divider/timer) can adjust here.
    virtual void setDoubleSpeed(bool /*doubleSpeed*/) {}

    /// Selects CGB or DMG compatibility behavior for model-specific registers.
    virtual void setCgb(bool /*cgb*/) {}

    /// CPU entered DMG compatibility mode via KEY0.  CGB hardware registers
    /// remain accessible, but WRAM banking follows DMG behaviour.
    virtual void setDmgCompatibilityMode(bool /*enabled*/) {}

    /// DMG OAM corruption hooks.
    virtual void triggerOamBugWrite(uint16_t /*address*/, int /*rowOffset*/ = 0) {}
    virtual void triggerOamBugRead(uint16_t /*address*/) {}
    virtual void triggerOamBugReadIncrease(uint16_t /*address*/) {}
};

}  // namespace qnemu