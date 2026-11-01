/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <span>

namespace qnemu
{

/**
 * Bundled Bootix DMG boot ROM (CC0).
 *
 * The 256-byte image is generated from third_party/bootix/bootroms/dmg_boot.bin
 * at CMake configure time and maps only 0000-00FF.
 */
std::span<const uint8_t> dmgBootRom();

}  // namespace qnemu
