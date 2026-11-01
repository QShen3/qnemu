/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/ui/LaunchPolicy.h"

namespace qnemu
{

std::optional<GbPlatform> launchPlatformFromString(std::string_view value)
{
    if (value == "gb") {
        return GbPlatform::gb;
    }
    if (value == "sgb") {
        return GbPlatform::sgb;
    }
    if (value == "gbc") {
        return GbPlatform::gbc;
    }
    return std::nullopt;
}

GbPlatform selectLaunchPlatform(const GbCartridgeHeader& header,
    GbPlatform dmgOnlyPlatform,
    GbPlatform dualModePlatform,
    std::optional<GbPlatform> overridePlatform)
{
    if (header.cgbOnly) {
        return GbPlatform::gbc;
    }
    if (overridePlatform) {
        return *overridePlatform;
    }
    return header.cgb ? dualModePlatform : dmgOnlyPlatform;
}

}  // namespace qnemu
