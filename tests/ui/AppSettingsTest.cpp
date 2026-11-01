/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryDir>

#include "qnemu/ui/AppSettings.h"

namespace qnemuTest
{

class AppSettingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        QCoreApplication::setOrganizationName(QStringLiteral("QNEmuTest"));
        QCoreApplication::setApplicationName(QStringLiteral("QNEmuTest"));
        QStandardPaths::setTestModeEnabled(true);
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
            settingsDirectory.path());
    }

    QTemporaryDir settingsDirectory;
};

TEST_F(AppSettingsTest, UsesPlatformDefaults)
{
    qnemu::AppSettings settings;

    EXPECT_EQ(settings.dmgOnlyLaunchPlatform(), QStringLiteral("gb"));
    EXPECT_EQ(settings.dualModeLaunchPlatform(), QStringLiteral("gbc"));
}

TEST_F(AppSettingsTest, PersistsDmgOnlyLaunchPlatform)
{
    qnemu::AppSettings settings;
    settings.setDmgOnlyLaunchPlatform(QStringLiteral("sgb"));

    QSettings stored;
    EXPECT_EQ(stored.value(QStringLiteral("platforms/launch/dmgOnly")).toString(),
        QStringLiteral("sgb"));
}

TEST_F(AppSettingsTest, PersistsDualModeLaunchPlatform)
{
    qnemu::AppSettings settings;
    settings.setDualModeLaunchPlatform(QStringLiteral("gb"));

    QSettings stored;
    EXPECT_EQ(stored.value(QStringLiteral("platforms/launch/dualMode")).toString(),
        QStringLiteral("gb"));
}

TEST_F(AppSettingsTest, RejectsInvalidLaunchPlatform)
{
    qnemu::AppSettings settings;
    settings.setDmgOnlyLaunchPlatform(QStringLiteral("cgb"));
    settings.setDualModeLaunchPlatform(QStringLiteral("dmg"));

    EXPECT_EQ(settings.dmgOnlyLaunchPlatform(), QStringLiteral("gb"));
    EXPECT_EQ(settings.dualModeLaunchPlatform(), QStringLiteral("gbc"));
}

}  // namespace qnemuTest
