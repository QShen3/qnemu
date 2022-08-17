/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbInterruptHandlerInterface : public GbDeviceInterface
{
public:
    virtual void requestVBlankInterrupt() = 0;
    virtual void requestLcdInterrupt() = 0;
    virtual void requestTimerInterrupt() = 0;
    virtual void requestSerialInterrupt() = 0;
    virtual void requestJoypadInterrupt() = 0;
};

}  // namespace qnemu
