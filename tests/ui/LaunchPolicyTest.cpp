/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <gtest/gtest.h>

#include "qnemu/gb/cartridge/GbCartridgeHeader.h"
#include "qnemu/ui/LaunchPolicy.h"

namespace qnemuTest
{

using qnemu::GbCartridgeHeader;
using qnemu::GbPlatform;
using qnemu::launchPlatformFromString;
using qnemu::selectLaunchPlatform;

namespace
{

GbCartridgeHeader cartridge(bool cgb, bool cgbOnly)
{
    GbCartridgeHeader header;
    header.cgb = cgb;
    header.cgbOnly = cgbOnly;
    return header;
}

}  // namespace

TEST(LaunchPolicyTest, ParsesPlatformNames)
{
    EXPECT_EQ(launchPlatformFromString("gb"), GbPlatform::gb);
    EXPECT_EQ(launchPlatformFromString("sgb"), GbPlatform::sgb);
    EXPECT_EQ(launchPlatformFromString("gbc"), GbPlatform::gbc);
    EXPECT_FALSE(launchPlatformFromString("dmg").has_value());
    EXPECT_FALSE(launchPlatformFromString("").has_value());
}

TEST(LaunchPolicyTest, DmgOnlyUsesGreyPreference)
{
    EXPECT_EQ(selectLaunchPlatform(cartridge(false, false),
                  GbPlatform::sgb, GbPlatform::gbc),
        GbPlatform::sgb);
}

TEST(LaunchPolicyTest, DualModeUsesDualModePreference)
{
    EXPECT_EQ(selectLaunchPlatform(cartridge(true, false),
                  GbPlatform::gb, GbPlatform::gbc),
        GbPlatform::gbc);
}

TEST(LaunchPolicyTest, CgbOnlyAlwaysUsesGbc)
{
    EXPECT_EQ(selectLaunchPlatform(cartridge(true, true),
                  GbPlatform::gb, GbPlatform::gb, GbPlatform::gb),
        GbPlatform::gbc);
}

TEST(LaunchPolicyTest, CommandLineOverrideWinsForDmgAndDualMode)
{
    EXPECT_EQ(selectLaunchPlatform(cartridge(false, false),
                  GbPlatform::gb, GbPlatform::gbc, GbPlatform::gbc),
        GbPlatform::gbc);
    EXPECT_EQ(selectLaunchPlatform(cartridge(true, false),
                  GbPlatform::gbc, GbPlatform::gbc, GbPlatform::sgb),
        GbPlatform::sgb);
}

TEST(LaunchPolicyTest, CommandLineOverrideCannotUnlockCgbOnly)
{
    EXPECT_EQ(selectLaunchPlatform(cartridge(true, true),
                  GbPlatform::gb, GbPlatform::gb, GbPlatform::sgb),
        GbPlatform::gbc);
}

}  // namespace qnemuTest
