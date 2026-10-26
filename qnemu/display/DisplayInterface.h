/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <functional>

#include "qnemu/display/FrameBuffer.h"

namespace qnemu
{

/**
 * Output side of the emulator.  The PPU only pushes a finished frame here;
 * it never touches QImage or waits for the window system.
 */
class DisplayInterface
{
public:
    virtual ~DisplayInterface() = default;

    virtual void presentFrame(const FrameBuffer& frame) = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;

    virtual void setKeyPressCallback(std::function<void(int)> callback) = 0;
    virtual void setKeyReleaseCallback(std::function<void(int)> callback) = 0;
};

}  // namespace qnemu
