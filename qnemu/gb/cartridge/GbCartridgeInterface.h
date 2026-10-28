/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/save/SaveTarget.h"

namespace qnemu
{

class GbCartridgeInterface : public GbDeviceInterface, public SaveTarget
{
public:
    virtual void load(const char* filePath) = 0;
    virtual bool isGbcCartridge() const = 0;
};

}  // namespace qnemu