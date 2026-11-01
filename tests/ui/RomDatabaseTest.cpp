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
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QTemporaryDir>

#include "qnemu/ui/RomDatabase.h"

namespace qnemuTest
{

namespace
{

QString writeFile(const QString& path, const QByteArray& data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    if (file.write(data) != data.size()) {
        return {};
    }
    return path;
}

QString writeDatabase(const QString& path, const QByteArray& rom, int schema = 1,
    const QString& status = QStringLiteral("official"))
{
    const QByteArray sha1 =
        QCryptographicHash::hash(rom, QCryptographicHash::Sha1).toHex().toUpper();
    const QByteArray md5 =
        QCryptographicHash::hash(rom, QCryptographicHash::Md5).toHex().toUpper();
    const quint32 crc = qnemu::RomDatabase::crc32(rom);

    const QJsonObject entry {
        {QStringLiteral("title"), QStringLiteral("Test Game")},
        {QStringLiteral("name"), QStringLiteral("Test Game (USA, Europe)")},
        {QStringLiteral("platform"), QStringLiteral("GB")},
        {QStringLiteral("region"), QStringLiteral("USA, Europe")},
        {QStringLiteral("languages"), QJsonArray {QStringLiteral("En"), QStringLiteral("Fr")}},
        {QStringLiteral("languagesInferred"), false},
        {QStringLiteral("developer"), QStringLiteral("Test Developer")},
        {QStringLiteral("publisher"), QStringLiteral("Test Publisher")},
        {QStringLiteral("genre"), QStringLiteral("Puzzle")},
        {QStringLiteral("year"), 1990},
        {QStringLiteral("serial"), QStringLiteral("DMG-TST")},
        {QStringLiteral("size"), rom.size()},
        {QStringLiteral("crc32"), QString::fromLatin1(
             QByteArray::number(crc, 16).rightJustified(8, '0').toUpper())},
        {QStringLiteral("md5"), QString::fromLatin1(md5)},
        {QStringLiteral("sha1"), QString::fromLatin1(sha1)},
        {QStringLiteral("status"), status},
    };
    const QJsonObject root {
        {QStringLiteral("schema"), schema},
        {QStringLiteral("source"), QStringLiteral("Test Database")},
        {QStringLiteral("sourceUrl"), QStringLiteral("https://example.invalid")},
        {QStringLiteral("sourceCommit"), QStringLiteral("test")},
        {QStringLiteral("sourceLicense"), QStringLiteral("CC BY-SA 4.0")},
        {QStringLiteral("sourceLicenseUrl"),
            QStringLiteral("https://creativecommons.org/licenses/by-sa/4.0/")},
        {QStringLiteral("datVersions"), QJsonObject {{QStringLiteral("GB"), QStringLiteral("test")}}},
        {QStringLiteral("generated"), QStringLiteral("2026-11-01")},
        {QStringLiteral("entries"), QJsonArray {entry}},
    };
    return writeFile(path, QJsonDocument(root).toJson(QJsonDocument::Compact));
}

}  // namespace

TEST(RomDatabaseTest, HashesFilesWithCrc32AndSha1)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QByteArray content("123456789");
    const QString path = writeFile(directory.filePath(QStringLiteral("test.bin")), content);
    ASSERT_FALSE(path.isEmpty());

    EXPECT_EQ(qnemu::RomDatabase::crc32(content), 0xCBF43926U);

    const qnemu::RomHash hash = qnemu::RomDatabase::hashFile(path);
    ASSERT_TRUE(hash.valid);
    EXPECT_EQ(hash.size, static_cast<quint64>(content.size()));
    EXPECT_EQ(hash.crc32, 0xCBF43926U);
    EXPECT_EQ(hash.sha1, QCryptographicHash::hash(content, QCryptographicHash::Sha1));
}

TEST(RomDatabaseTest, HashesFilesLargerThanOneChunk)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    QByteArray content((1 << 20) * 2 + 12345, 'q');
    for (qsizetype index = 0; index < content.size(); index += 4093) {
        content[index] = static_cast<char>(index & 0xFF);
    }
    const QString path = writeFile(directory.filePath(QStringLiteral("large.bin")), content);
    ASSERT_FALSE(path.isEmpty());

    const qnemu::RomHash hash = qnemu::RomDatabase::hashFile(path);
    ASSERT_TRUE(hash.valid);
    EXPECT_EQ(hash.size, static_cast<quint64>(content.size()));
    EXPECT_EQ(hash.crc32, qnemu::RomDatabase::crc32(content));
    EXPECT_EQ(hash.sha1, QCryptographicHash::hash(content, QCryptographicHash::Sha1));
}

TEST(RomDatabaseTest, HashesMemoryData)
{
    const QByteArray content("123456789");
    const qnemu::RomHash hash = qnemu::RomDatabase::hashData(content);
    ASSERT_TRUE(hash.valid);
    EXPECT_EQ(hash.size, static_cast<quint64>(content.size()));
    EXPECT_EQ(hash.crc32, 0xCBF43926U);
    EXPECT_EQ(hash.sha1, QCryptographicHash::hash(content, QCryptographicHash::Sha1));
}

TEST(RomDatabaseTest, RejectsMissingFiles)
{
    const qnemu::RomHash hash =
        qnemu::RomDatabase::hashFile(QStringLiteral("/nonexistent/qnemu-test-rom"));
    EXPECT_FALSE(hash.valid);
}

TEST(RomDatabaseTest, LoadsEntriesAndFindsBySha1)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QByteArray rom("123456789");
    const QString databasePath = writeDatabase(
        directory.filePath(QStringLiteral("romdb.json")), rom);
    ASSERT_FALSE(databasePath.isEmpty());

    qnemu::RomDatabase database;
    QString error;
    ASSERT_TRUE(database.load(databasePath, &error)) << error.toStdString();

    EXPECT_EQ(database.count(), 1);
    EXPECT_EQ(database.source(), QStringLiteral("Test Database"));
    EXPECT_EQ(database.sourceLicense(), QStringLiteral("CC BY-SA 4.0"));
    EXPECT_EQ(database.sourceLicenseUrl(),
        QStringLiteral("https://creativecommons.org/licenses/by-sa/4.0/"));
    EXPECT_EQ(database.datVersion(), QStringLiteral("GB test"));
    EXPECT_TRUE(database.hasSize(rom.size()));
    EXPECT_FALSE(database.hasSize(1));

    const QByteArray sha1 = QCryptographicHash::hash(rom, QCryptographicHash::Sha1);
    const qnemu::RomRecord* record = database.find(sha1);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->title, QStringLiteral("Test Game"));
    EXPECT_EQ(record->name, QStringLiteral("Test Game (USA, Europe)"));
    EXPECT_EQ(record->platform, QStringLiteral("GB"));
    EXPECT_EQ(record->region, QStringLiteral("USA, Europe"));
    EXPECT_EQ(record->languages,
        QStringList({QStringLiteral("En"), QStringLiteral("Fr")}));
    EXPECT_FALSE(record->languagesInferred);
    EXPECT_EQ(record->developer, QStringLiteral("Test Developer"));
    EXPECT_EQ(record->publisher, QStringLiteral("Test Publisher"));
    EXPECT_EQ(record->genre, QStringLiteral("Puzzle"));
    EXPECT_EQ(record->year, 1990);
    EXPECT_EQ(record->serial, QStringLiteral("DMG-TST"));
    EXPECT_EQ(record->size, static_cast<quint32>(rom.size()));
    EXPECT_EQ(record->crc32, 0xCBF43926U);
    EXPECT_EQ(record->sha1, sha1);
    EXPECT_EQ(record->md5.size(), 16);
    EXPECT_EQ(record->status, QStringLiteral("official"));
    EXPECT_TRUE(record->official());

    EXPECT_EQ(database.find(QByteArray(20, '\0')), nullptr);
}

TEST(RomDatabaseTest, RejectsUnsupportedSchema)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString databasePath = writeDatabase(
        directory.filePath(QStringLiteral("romdb.json")), QByteArray("123456789"), 2);
    ASSERT_FALSE(databasePath.isEmpty());

    qnemu::RomDatabase database;
    QString error;
    EXPECT_FALSE(database.load(databasePath, &error));
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(database.count(), 0);
}

TEST(RomDatabaseTest, NonOfficialEntriesAreNotOfficial)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QByteArray rom("123456789");
    const QString databasePath = writeDatabase(
        directory.filePath(QStringLiteral("romdb.json")), rom, 1, QStringLiteral("proto"));
    ASSERT_FALSE(databasePath.isEmpty());

    qnemu::RomDatabase database;
    QString error;
    ASSERT_TRUE(database.load(databasePath, &error)) << error.toStdString();

    const qnemu::RomRecord* record =
        database.find(QCryptographicHash::hash(rom, QCryptographicHash::Sha1));
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->status, QStringLiteral("proto"));
    EXPECT_FALSE(record->official());
}

TEST(RomDatabaseTest, LoadsShippedDatabase)
{
    qnemu::RomDatabase database;
    QString error;
    ASSERT_TRUE(database.load(QStringLiteral(QNEMU_ROMDB_PATH), &error)) << error.toStdString();

    EXPECT_GT(database.count(), 4000);
    EXPECT_EQ(database.source(), QStringLiteral("libretro-database (No-Intro)"));
    EXPECT_EQ(database.sourceLicense(), QStringLiteral("CC BY-SA 4.0"));
    EXPECT_EQ(database.sourceLicenseUrl(),
        QStringLiteral("https://creativecommons.org/licenses/by-sa/4.0/"));
    EXPECT_TRUE(database.datVersion().contains(QStringLiteral("GB")));
    EXPECT_TRUE(database.datVersion().contains(QStringLiteral("GBC")));

    const QByteArray sha1 = QByteArray::fromHex("74591CC9501AF93873F9A5D3EB12DA12C0723BBC");
    const qnemu::RomRecord* record = database.find(sha1);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->title, QStringLiteral("Tetris"));
    EXPECT_EQ(record->publisher, QStringLiteral("Nintendo"));
    EXPECT_EQ(record->developer, QStringLiteral("Nintendo"));
    EXPECT_EQ(record->region, QStringLiteral("World"));
    EXPECT_EQ(record->crc32, 0x46DF91ADU);
    EXPECT_TRUE(record->official());
}

}  // namespace qnemuTest
