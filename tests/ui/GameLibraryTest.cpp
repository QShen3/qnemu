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
#include <QtCore/QFile>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>

#include "qnemu/ui/GameLibrary.h"

namespace qnemuTest
{

namespace
{

QString writeRom(const QTemporaryDir& directory, const QString& name,
    const QByteArray& title, uint8_t cgbFlag)
{
    const QString path = directory.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    QByteArray data(0x150, 0);
    data.replace(0x134, title.size(), title);
    data[0x143] = static_cast<char>(cgbFlag);
    data[0x147] = static_cast<char>(0x00);
    data[0x14A] = static_cast<char>(0x01);
    data[0x14B] = static_cast<char>(0x31);
    if (file.write(data) != data.size()) {
        return {};
    }
    return path;
}

}  // namespace

class GameLibraryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        QCoreApplication::setOrganizationName(QStringLiteral("QNEmuTest"));
        QCoreApplication::setApplicationName(QStringLiteral("QNEmuTest"));
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
            settingsDirectory.path());
    }

    QTemporaryDir settingsDirectory;
};

TEST_F(GameLibraryTest, ClassifiesByCartridgeHeaderNotExtension)
{
    QTemporaryDir roms;
    ASSERT_TRUE(roms.isValid());
    ASSERT_FALSE(writeRom(roms, QStringLiteral("dmg.gb"), QByteArray("DMG!"), 0x00).isEmpty());
    ASSERT_FALSE(writeRom(roms, QStringLiteral("dual.gbc"), QByteArray("DUAL"), 0x80).isEmpty());
    ASSERT_FALSE(writeRom(roms, QStringLiteral("cgb.gbc"), QByteArray("CGB!"), 0xC0).isEmpty());

    qnemu::GameLibrary library;
    library.setPaths(roms.path(), roms.path());

    EXPECT_EQ(library.gbCount(), 2);
    EXPECT_EQ(library.gbcCount(), 1);

    QStringList gbTitles;
    for (int row = 0; row < library.gbCount(); ++row) {
        gbTitles.append(library.gbGames()->data(library.gbGames()->index(row), Qt::DisplayRole).toString());
    }
    EXPECT_TRUE(gbTitles.contains(QStringLiteral("DMG!")));
    EXPECT_TRUE(gbTitles.contains(QStringLiteral("DUAL")));
    EXPECT_FALSE(gbTitles.contains(QStringLiteral("CGB!")));

    const QVariant title = library.gbcGames()->data(
        library.gbcGames()->index(0), Qt::DisplayRole);
    EXPECT_EQ(title.toString(), QStringLiteral("CGB!"));
    const QVariant platform = library.gbcGames()->data(
        library.gbcGames()->index(0), qnemu::GameListModel::PlatformRole);
    EXPECT_EQ(platform.toString(), QStringLiteral("GBC"));
}

}  // namespace qnemuTest
