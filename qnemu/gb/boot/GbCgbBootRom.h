/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <span>

namespace qnemu
{

/**
 * Bundled QNEMU CGB boot ROM.
 *
 * The 2304-byte image is generated from
 * third_party/sameboy/bootroms/cgb_boot.bin at CMake configure time. It uses
 * the standard CGB layout: $0000-$00FF, the $0100-$01FF cartridge-header gap,
 * then $0200-$08FF.
 */
std::span<const uint8_t> cgbBootRom();

}  // namespace qnemu
