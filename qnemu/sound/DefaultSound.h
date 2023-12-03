/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

class DefaultSound : public SoundInterface
{

public:
    DefaultSound() = default;
    ~DefaultSound() = default;

    void enable() override;
    void disable() override;

private:
    bool isEnabled;
};

}  // namespace qnemu