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

    void presentSgbFrame(const qnemu::SgbFrameBuffer& frame) override
    {
        lastSgbFrame = frame;
        ++sgbFrameCount;
    }

    void enable() override { enabled = true; }
    void disable() override { enabled = false; }

    qnemu::FrameBuffer lastFrame;
    qnemu::SgbFrameBuffer lastSgbFrame;
    uint64_t frameCount = 0;
    uint64_t sgbFrameCount = 0;
    bool enabled = true;
};

}  // namespace qnemuTest
