/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

namespace qnemu
{

/// The three CGB palettes initialized for a DMG cartridge running on a GBC:
/// BG palette 0 and OBJ palettes 0/1.  Colors are RGB555 values.
struct DmgCompatibilityPalette
{
    std::array<uint16_t, 4> object0 {};
    std::array<uint16_t, 4> object1 {};
    std::array<uint16_t, 4> background {};
};

/// Header bytes needed by the CGB boot ROM's compatibility palette algorithm.
struct CgbCompatibilityHeader
{
    std::array<uint8_t, 16> title {};
    uint8_t oldLicensee = 0;
    std::array<uint8_t, 2> newLicensee {};
};

/// Selects the per-game CGB compatibility palette for a DMG cartridge.
///
/// The algorithm follows the CGB boot ROM: only Nintendo-licensed headers take
/// part in the title checksum lookup; duplicate checksums are disambiguated by
/// the fourth title character; unknown or non-Nintendo games use combination 0.
DmgCompatibilityPalette cgbCompatibilityPalette(const CgbCompatibilityHeader& header);

}  // namespace qnemu
