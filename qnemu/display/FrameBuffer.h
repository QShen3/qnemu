/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace qnemu
{

inline constexpr uint32_t rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    return 0xFF000000U
        | (static_cast<uint32_t>(red) << 16)
        | (static_cast<uint32_t>(green) << 8)
        | static_cast<uint32_t>(blue);
}

/**
 * Plain 160x144 RGB32 framebuffer.
 *
 * Pixels are stored as 0xFFRRGGBB, which maps directly to QImage's
 * Format_RGB32 on little-endian hosts but keeps the PPU core free of Qt.
 */
class FrameBuffer
{
public:
    static constexpr int width = 160;
    static constexpr int height = 144;
    static constexpr std::size_t pixelCount = static_cast<std::size_t>(width * height);

    uint32_t& at(int x, int y)
    {
        return pixels.at(static_cast<std::size_t>(y * width + x));
    }

    const uint32_t& at(int x, int y) const
    {
        return pixels.at(static_cast<std::size_t>(y * width + x));
    }

    void fill(uint32_t color)
    {
        pixels.fill(color);
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::array<uint32_t, pixelCount> pixels{};
};

}  // namespace qnemu
