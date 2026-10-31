/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace qnemu
{

/**
 * 256x224 SGB output frame.  The Game Boy picture occupies the central
 * 160x144 area; the surrounding pixels come from the SGB border.
 */
class SgbFrameBuffer
{
public:
    static constexpr int width = 256;
    static constexpr int height = 224;
    static constexpr std::size_t pixelCount = static_cast<std::size_t>(width * height);

    uint32_t& at(int x, int y)
    {
        return pixels.at(static_cast<std::size_t>(y * width + x));
    }

    const uint32_t& at(int x, int y) const
    {
        return pixels.at(static_cast<std::size_t>(y * width + x));
    }

    std::array<uint32_t, pixelCount> pixels {};
};

}  // namespace qnemu
