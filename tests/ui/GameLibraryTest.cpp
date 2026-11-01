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

#include <QtCore/QCryptographicHash>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QSettings>
#include <QtCore/QTemporaryDir>

#include "qnemu/ui/GameLibrary.h"
#include "qnemu/ui/RomDatabase.h"
#include "tests/support/TestArchive.h"

namespace qnemuTest
{

namespace
{

QByteArray makeRom(const QByteArray& title, uint8_t cgbFlag)
{
    QByteArray data(0x150, 0);
    data.replace(0x134, title.size(), title);
    data[0x143] = static_cast<char>(cgbFlag);
    data[0x147] = static_cast<char>(0x00);
    data[0x14A] = static_cast<char>(0x01);
    data[0x14B] = static_cast<char>(0x31);
    return data;
}

QString writeRom(const QTemporaryDir& directory, const QString& name,
    const QByteArray& title, uint8_t cgbFlag)
{
    const QString path = directory.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    const QByteArray data = makeRom(title, cgbFlag);
    if (file.write(data) != data.size()) {
        return {};
    }
    return path;
}

QString writeDatabase(const QByteArray& data, const QString& databasePath)
{
    const QByteArray sha1 =
        QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex().toUpper();
    const QByteArray md5 =
        QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex().toUpper();
    const quint32 crc = qnemu::RomDatabase::crc32(data);

    const QJsonObject entry {
        {QStringLiteral("title"), QStringLiteral("Tetris")},
        {QStringLiteral("name"), QStringLiteral("Tetris (World) (Rev 1)")},
        {QStringLiteral("platform"), QStringLiteral("GB")},
        {QStringLiteral("region"), QStringLiteral("World")},
        {QStringLiteral("languages"), QJsonArray {QStringLiteral("En")}},
        {QStringLiteral("languagesInferred"), true},
        {QStringLiteral("developer"), QStringLiteral("Nintendo")},
        {QStringLiteral("publisher"), QStringLiteral("Nintendo")},
        {QStringLiteral("genre"), QStringLiteral("Puzzle")},
        {QStringLiteral("year"), 1989},
        {QStringLiteral("serial"), QStringLiteral("DMG-TRA")},
        {QStringLiteral("size"), data.size()},
        {QStringLiteral("crc32"), QString::fromLatin1(
             QByteArray::number(crc, 16).rightJustified(8, '0').toUpper())},
        {QStringLiteral("md5"), QString::fromLatin1(md5)},
        {QStringLiteral("sha1"), QString::fromLatin1(sha1)},
        {QStringLiteral("status"), QStringLiteral("official")},
    };
    const QJsonObject root {
        {QStringLiteral("schema"), 1},
        {QStringLiteral("source"), QStringLiteral("Test Database")},
        {QStringLiteral("sourceUrl"), QStringLiteral("https://example.invalid")},
        {QStringLiteral("sourceCommit"), QStringLiteral("test")},
        {QStringLiteral("sourceLicense"), QStringLiteral("CC BY-SA 4.0")},
        {QStringLiteral("datVersions"),
            QJsonObject {{QStringLiteral("GB"), QStringLiteral("test")}}},
        {QStringLiteral("generated"), QStringLiteral("2026-11-01")},
        {QStringLiteral("entries"), QJsonArray {entry}},
    };

    QFile file(databasePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Compact);
    if (file.write(json) != json.size()) {
        return {};
    }
    return databasePath;
}

QString writeDatabase(const QString& romPath, const QString& databasePath)
{
    QFile rom(romPath);
    if (!rom.open(QIODevice::ReadOnly)) {
        return {};
    }
    return writeDatabase(rom.readAll(), databasePath);
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

    qnemu::GameLibrary library(nullptr, QString());
    library.setPaths(roms.path(), roms.path());
    library.waitForScan();

    EXPECT_EQ(library.gbCount(), 2);
    EXPECT_EQ(library.gbcCount(), 1);

    QStringList gbTitles;
    for (int row = 0; row < library.gbCount(); ++row) {
        gbTitles.append(library.gbGames()->data(library.gbGames()->index(row, 0), Qt::DisplayRole).toString());
    }
    EXPECT_TRUE(gbTitles.contains(QStringLiteral("DMG!")));
    EXPECT_TRUE(gbTitles.contains(QStringLiteral("DUAL")));
    EXPECT_FALSE(gbTitles.contains(QStringLiteral("CGB!")));

    const QVariant title = library.gbcGames()->data(
        library.gbcGames()->index(0, 0), Qt::DisplayRole);
    EXPECT_EQ(title.toString(), QStringLiteral("CGB!"));
    const QVariant platform = library.gbcGames()->data(
        library.gbcGames()->index(0, 0), qnemu::GameListModel::PlatformRole);
    EXPECT_EQ(platform.toString(), QStringLiteral("GBC"));
}

TEST_F(GameLibraryTest, MarksOfficialRomFromBuiltInDatabase)
{
    QTemporaryDir roms;
    ASSERT_TRUE(roms.isValid());
    const QString romPath = writeRom(roms, QStringLiteral("tetris.gb"),
        QByteArray("TETRIS"), 0x00);
    ASSERT_FALSE(romPath.isEmpty());

    const QString databasePath = roms.filePath(QStringLiteral("romdb.json"));
    ASSERT_FALSE(writeDatabase(romPath, databasePath).isEmpty());

    qnemu::GameLibrary library(nullptr, databasePath);
    library.setPaths(roms.path(), roms.path());
    library.waitForScan();

    ASSERT_EQ(library.gbCount(), 1);
    ASSERT_EQ(library.gbcCount(), 0);
    EXPECT_EQ(library.databaseCount(), 1);
    EXPECT_EQ(library.databaseSource(), QStringLiteral("Test Database GB test"));

    const QModelIndex index = library.gbGames()->index(0, 0);
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::TitleRole).toString(),
        QStringLiteral("Tetris"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::PublisherRole).toString(),
        QStringLiteral("Nintendo"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::DeveloperRole).toString(),
        QStringLiteral("Nintendo"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::RegionRole).toString(),
        QStringLiteral("World"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::LanguagesRole).toString(),
        QStringLiteral("English"));
    EXPECT_TRUE(library.gbGames()->data(index, qnemu::GameListModel::LanguagesInferredRole).toBool());
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::SerialRole).toString(),
        QStringLiteral("DMG-TRA"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::GenreRole).toString(),
        QStringLiteral("Puzzle"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::YearRole).toInt(), 1989);
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::StatusRole).toString(),
        QStringLiteral("official"));
    EXPECT_TRUE(library.gbGames()->data(index, qnemu::GameListModel::OfficialRole).toBool());
    EXPECT_TRUE(library.gbGames()->data(index, qnemu::GameListModel::VerifiedRole).toBool());
    EXPECT_FALSE(library.gbGames()->data(index, qnemu::GameListModel::CrcRole).toString().isEmpty());
    EXPECT_FALSE(library.gbGames()->data(index, qnemu::GameListModel::Sha1Role).toString().isEmpty());
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::DatabaseNameRole).toString(),
        QStringLiteral("Tetris (World) (Rev 1)"));
}

TEST_F(GameLibraryTest, MarksOfficialRomInsideZip)
{
    QTemporaryDir roms;
    ASSERT_TRUE(roms.isValid());
    const QByteArray rom = makeRom(QByteArray("ZIPGAME"), 0x00);
    const QString zipPath = roms.filePath(QStringLiteral("tetris.zip"));
    ASSERT_TRUE(writeTestArchive(zipPath.toStdString(), {
        {"tetris.gb", std::vector<uint8_t>(rom.begin(), rom.end())},
    }));

    const QString databasePath = roms.filePath(QStringLiteral("romdb.json"));
    ASSERT_FALSE(writeDatabase(rom, databasePath).isEmpty());

    qnemu::GameLibrary library(nullptr, databasePath);
    library.setPaths(roms.path(), roms.path());
    library.waitForScan();

    ASSERT_EQ(library.gbCount(), 1);
    ASSERT_EQ(library.gbcCount(), 0);
    const QModelIndex index = library.gbGames()->index(0, 0);
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::TitleRole).toString(),
        QStringLiteral("Tetris"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::ArchiveEntryRole).toString(),
        QStringLiteral("tetris.gb"));
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::StatusRole).toString(),
        QStringLiteral("official"));
    EXPECT_TRUE(library.gbGames()->data(index, qnemu::GameListModel::OfficialRole).toBool());
    EXPECT_TRUE(library.gbGames()->data(index, qnemu::GameListModel::VerifiedRole).toBool());
    EXPECT_EQ(library.gbGames()->data(index, qnemu::GameListModel::PlatformRole).toString(),
        QStringLiteral("GB"));

    // A second scan should use the cached archive header/hash instead of
    // decompressing the zip again.
    library.refresh();
    library.waitForScan();
    ASSERT_EQ(library.gbCount(), 1);
    const QModelIndex refreshed = library.gbGames()->index(0, 0);
    EXPECT_EQ(library.gbGames()->data(refreshed, qnemu::GameListModel::ArchiveEntryRole).toString(),
        QStringLiteral("tetris.gb"));
    EXPECT_TRUE(library.gbGames()->data(refreshed, qnemu::GameListModel::OfficialRole).toBool());
}

TEST_F(GameLibraryTest, FiltersByTitleAndTerms)
{
    QTemporaryDir roms;
    ASSERT_TRUE(roms.isValid());
    ASSERT_FALSE(writeRom(roms, QStringLiteral("mario.gb"),
        QByteArray("MARIO"), 0x00).isEmpty());
    ASSERT_FALSE(writeRom(roms, QStringLiteral("zelda.gb"),
        QByteArray("ZELDA"), 0x00).isEmpty());

    qnemu::GameLibrary library(nullptr, QString());
    library.setPaths(roms.path(), roms.path());
    library.waitForScan();
    EXPECT_EQ(library.gbCount(), 2);

    library.setFilterText(QStringLiteral("mario"));
    EXPECT_EQ(library.filterText(), QStringLiteral("mario"));
    ASSERT_EQ(library.gbCount(), 1);
    EXPECT_EQ(library.gbGames()->data(library.gbGames()->index(0, 0),
                  qnemu::GameListModel::TitleRole).toString(),
        QStringLiteral("MARIO"));

    library.setFilterText(QStringLiteral("mario zelda"));
    EXPECT_EQ(library.gbCount(), 0);

    library.setFilterText(QString());
    EXPECT_EQ(library.gbCount(), 2);
}

}  // namespace qnemuTest
