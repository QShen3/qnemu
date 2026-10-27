/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "qnemu/sound/SoundInterface.h"

namespace qnemuTest
{

/// Captures the stereo frames the APU hands to the sound backend.
class FakeSound : public qnemu::SoundInterface
{
public:
    void enable() override { enabled = true; }
    void disable() override { enabled = false; }

    void addSamples(const int16_t* samples, std::size_t frameCount) override
    {
        for (std::size_t i = 0; i < frameCount; ++i) {
            frames.emplace_back(samples[i * 2], samples[i * 2 + 1]);
        }
    }

    std::vector<std::pair<int16_t, int16_t>> frames;
    bool enabled = true;
};

}  // namespace qnemuTest
