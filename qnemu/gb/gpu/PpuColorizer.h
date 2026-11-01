/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

/**
 * Optional DMG pixel colorizer.
 *
 * A plain DMG uses BGP/OBP and the CGB compatibility palette directly.  SGB
 * supplies this interface to replace the shade lookup with its attribute
 * palette, without making the PPU depend on the SGB implementation.
 */
class PpuColorizer
{
public:
    virtual ~PpuColorizer() = default;

    virtual uint32_t colorForDmgPixel(int x, int y,
        uint8_t colorIndex, uint8_t palette) const = 0;
};

}  // namespace qnemu
