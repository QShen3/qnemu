/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/display/FrameBuffer.h"

namespace qnemuTest
{

class MemoryDisplay : public qnemu::DisplayInterface
{
public:
    void presentFrame(const qnemu::FrameBuffer& frame) override
    {
        lastFrame = frame;
        ++frameCount;
    }

    void enable() override { enabled = true; }
    void disable() override { enabled = false; }

    qnemu::FrameBuffer lastFrame;
    uint64_t frameCount = 0;
    bool enabled = true;
};

}  // namespace qnemuTest
