/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <optional>
#include <string_view>

#include "qnemu/gb/GbPlatform.h"
#include "qnemu/gb/cartridge/GbCartridgeHeader.h"

namespace qnemu
{

/// Parses the value stored by AppSettings (`gb`, `sgb` or `gbc`).
std::optional<GbPlatform> launchPlatformFromString(std::string_view value);

/**
 * Selects the hardware platform for a cartridge.
 *
 * CGB-only cartridges always force `gbc`; the per-type preferences only
 * apply to DMG-only and dual-mode cartridges.  A command-line override wins
 * over the preferences but never over the CGB-only constraint.
 */
GbPlatform selectLaunchPlatform(const GbCartridgeHeader& header,
    GbPlatform dmgOnlyPlatform,
    GbPlatform dualModePlatform,
    std::optional<GbPlatform> overridePlatform = std::nullopt);

}  // namespace qnemu
