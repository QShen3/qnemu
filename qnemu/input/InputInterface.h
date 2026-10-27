/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <functional>

#include "qnemu/gb/joypad/GbButton.h"

namespace qnemu
{

/**
 * Host input source.  Keeping this separate from DisplayInterface means the
 * joypad no longer needs to know about the window system (or Qt key codes),
 * and a future input backend (keyboard, gamepad, replay) only has to emit
 * GbButton transitions.
 */
class InputInterface
{
public:
    virtual ~InputInterface() = default;

    virtual void setButtonPressCallback(std::function<void(GbButton)> callback) = 0;
    virtual void setButtonReleaseCallback(std::function<void(GbButton)> callback) = 0;
};

}  // namespace qnemu
