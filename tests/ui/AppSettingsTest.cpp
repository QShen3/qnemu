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

TEST_F(AppSettingsTest, SystemAccelerometerDefaultsOnOnlyWhenAvailable)
{
    qnemu::AppSettings unavailable(nullptr, false);
    EXPECT_FALSE(unavailable.systemAccelerometerAvailable());
    EXPECT_FALSE(unavailable.useSystemAccelerometer());
    unavailable.setUseSystemAccelerometer(true);
    EXPECT_FALSE(unavailable.useSystemAccelerometer());

    qnemu::AppSettings available(nullptr, true);
    EXPECT_TRUE(available.systemAccelerometerAvailable());
    EXPECT_TRUE(available.useSystemAccelerometer());
    available.setUseSystemAccelerometer(false);
    EXPECT_FALSE(available.useSystemAccelerometer());

    qnemu::AppSettings restored(nullptr, true);
    EXPECT_FALSE(restored.useSystemAccelerometer());
}

TEST_F(AppSettingsTest, ExistingKeyProfilesGainTiltDefaults)
{
    QSettings stored;
    QVariantList profiles;
    profiles.append(QVariantMap {
        {QStringLiteral("name"), QStringLiteral("Old")},
        {QStringLiteral("bindings"), QVariantMap {
            {QStringLiteral("up"), static_cast<int>(Qt::Key_W)},
        }},
    });
    stored.setValue(QStringLiteral("keys/profiles"), profiles);
    stored.setValue(QStringLiteral("keys/activeProfile"), QStringLiteral("Old"));

    qnemu::AppSettings settings;
    const QVariantMap bindings = settings.activeKeyBindings();
    EXPECT_EQ(bindings.value(QStringLiteral("up")).toInt(), Qt::Key_W);
    EXPECT_EQ(bindings.value(QStringLiteral("tiltLeft")).toInt(), Qt::Key_Left);
    EXPECT_EQ(bindings.value(QStringLiteral("tiltRight")).toInt(), Qt::Key_Right);
    EXPECT_EQ(bindings.value(QStringLiteral("tiltUp")).toInt(), Qt::Key_Up);
    EXPECT_EQ(bindings.value(QStringLiteral("tiltDown")).toInt(), Qt::Key_Down);
}

TEST_F(AppSettingsTest, PersistsNamedSaveSlots)
{
    qnemu::AppSettings settings;
    EXPECT_TRUE(settings.saveSlotName(QStringLiteral("Pokemon Red"), 2).isEmpty());

    settings.setSaveSlotName(QStringLiteral("Pokemon Red"), 2,
        QStringLiteral("  Player 2  "));
    EXPECT_EQ(settings.saveSlotName(QStringLiteral("Pokemon Red"), 2),
        QStringLiteral("Player 2"));

    qnemu::AppSettings other;
    EXPECT_EQ(other.saveSlotName(QStringLiteral("Pokemon Red"), 2),
        QStringLiteral("Player 2"));

    other.setSaveSlotName(QStringLiteral("Pokemon Red"), 3,
        QStringLiteral("Fourth"));
    EXPECT_EQ(settings.saveSlotName(QStringLiteral("Pokemon Red"), 3),
        QStringLiteral("Fourth"));

    other.setSaveSlotName(QStringLiteral("Pokemon Red"), 2, QStringLiteral("   "));
    EXPECT_TRUE(settings.saveSlotName(QStringLiteral("Pokemon Red"), 2).isEmpty());
    EXPECT_EQ(settings.saveSlotName(QStringLiteral("Pokemon Red"), 0),
        QString());
}

}  // namespace qnemuTest
