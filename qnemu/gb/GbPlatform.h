/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

namespace qnemu
{

/**
 * Hardware platform a cartridge is launched on.
 *
 * This is deliberately separate from the cartridge type: a DMG-only or
 * dual-mode cartridge can start on any of the three platforms, while a
 * CGB-only cartridge can only start on `gbc`.
 */
enum class GbPlatform
{
    gb,
    sgb,
    gbc,
};

}  // namespace qnemu
