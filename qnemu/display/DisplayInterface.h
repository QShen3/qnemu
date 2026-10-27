/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/display/FrameBuffer.h"

namespace qnemu
{

/**
 * Output side of the emulator.  The PPU only pushes a finished frame here;
 * it never touches QImage or waits for the window system.  Host input is a
 * separate concern (see qnemu/input/InputInterface.h).
 */
class DisplayInterface
{
public:
    virtual ~DisplayInterface() = default;

    virtual void presentFrame(const FrameBuffer& frame) = 0;

    virtual void enable() = 0;
    virtual void disable() = 0;
};

}  // namespace qnemu
