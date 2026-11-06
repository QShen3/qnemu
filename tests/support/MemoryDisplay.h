/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>

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
        frameCount.fetch_add(1, std::memory_order_relaxed);
    }

    void presentSgbFrame(const qnemu::SgbFrameBuffer& frame) override
    {
        lastSgbFrame = frame;
        sgbFrameCount.fetch_add(1, std::memory_order_relaxed);
    }

    /// Waits until at least `count` frames have been presented.
    bool waitForFrames(uint64_t count,
        std::chrono::seconds timeout = std::chrono::seconds(180)) const
    {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (frameCount.load(std::memory_order_relaxed) < count) {
            if (std::chrono::steady_clock::now() >= deadline) {
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return true;
    }

    void enable() override { enabled = true; }
    void disable() override { enabled = false; }

    qnemu::FrameBuffer lastFrame;
    qnemu::SgbFrameBuffer lastSgbFrame;
    std::atomic<uint64_t> frameCount {0};
    std::atomic<uint64_t> sgbFrameCount {0};
    bool enabled = true;
};

}  // namespace qnemuTest
