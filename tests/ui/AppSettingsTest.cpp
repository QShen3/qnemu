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

TEST_F(AppSettingsTest, MigratesLegacyLaunchModel)
{
    {
        QSettings settings;
        settings.setValue(QStringLiteral("emulator/launchModel"), QStringLiteral("dmg"));
        settings.sync();
    }

    qnemu::AppSettings settings;
    EXPECT_EQ(settings.gbLaunchModel(), QStringLiteral("dmg"));

    QSettings stored;
    EXPECT_EQ(stored.value(QStringLiteral("platforms/gb/launchModel")).toString(),
        QStringLiteral("dmg"));
}

TEST_F(AppSettingsTest, PersistsGbLaunchModel)
{
    qnemu::AppSettings settings;
    settings.setGbLaunchModel(QStringLiteral("sgb"));

    QSettings stored;
    EXPECT_EQ(stored.value(QStringLiteral("platforms/gb/launchModel")).toString(),
        QStringLiteral("sgb"));
}

}  // namespace qnemuTest
