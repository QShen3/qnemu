/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <span>

namespace qnemu
{

/**
 * Bundled clean-room SGB boot ROM.
 *
 * The 256-byte image forwards the cartridge header to the SNES-side SGB
 * BIOS, then unmaps itself.  It is generated from
 * third_party/sgb-boot/bootroms/sgb_boot.bin at CMake configure time.
 */
std::span<const uint8_t> sgbBootRom();

}  // namespace qnemu
