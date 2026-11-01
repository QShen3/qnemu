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

#include <cstdint>
#include <vector>

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include "qnemu/ui/RomArchive.h"
#include "tests/support/TestArchive.h"

namespace qnemuTest
{

namespace
{

std::vector<uint8_t> makeRom()
{
    std::vector<uint8_t> data(0x150, 0);
    const char title[] = "ARCHIVE";
    for (std::size_t index = 0; index < sizeof(title) - 1; ++index) {
        data[0x134 + index] = static_cast<uint8_t>(title[index]);
    }
    data[0x143] = 0x00;
    data[0x147] = 0x00;
    data[0x14A] = 0x01;
    data[0x14B] = 0x31;
    return data;
}

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

}  // namespace

TEST(RomArchiveTest, DetectsArchiveSuffixes)
{
    EXPECT_TRUE(qnemu::RomArchive::isArchivePath(QStringLiteral("game.zip")));
    EXPECT_TRUE(qnemu::RomArchive::isArchivePath(QStringLiteral("game.7z")));
    EXPECT_TRUE(qnemu::RomArchive::isArchivePath(QStringLiteral("game.rar")));
    EXPECT_TRUE(qnemu::RomArchive::isArchivePath(QStringLiteral("GAME.ZIP")));
    EXPECT_FALSE(qnemu::RomArchive::isArchivePath(QStringLiteral("game.gb")));
    EXPECT_FALSE(qnemu::RomArchive::isArchivePath(QStringLiteral("game.gbc")));
}

TEST(RomArchiveTest, ReadsSingleDeflateRom)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("game.zip"));
    const std::vector<uint8_t> rom = makeRom();
    ASSERT_TRUE(writeTestArchive(path.toStdString(), {{"game.gb", rom}}, "deflate"));

    qnemu::RomArchiveEntry entry;
    QString error;
    ASSERT_TRUE(qnemu::RomArchive::read(path, &entry, &error)) << error.toStdString();
    EXPECT_EQ(entry.name, QStringLiteral("game.gb"));
    EXPECT_EQ(entry.data.size(), static_cast<qsizetype>(rom.size()));
    EXPECT_EQ(entry.data, QByteArray(reinterpret_cast<const char*>(rom.data()),
        static_cast<qsizetype>(rom.size())));
}

TEST(RomArchiveTest, ReadsStoredRom)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("stored.zip"));
    const std::vector<uint8_t> rom = makeRom();
    ASSERT_TRUE(writeTestArchive(path.toStdString(), {{"game.gbc", rom}}, "store"));

    qnemu::RomArchiveEntry entry;
    QString error;
    ASSERT_TRUE(qnemu::RomArchive::read(path, &entry, &error)) << error.toStdString();
    EXPECT_EQ(entry.name, QStringLiteral("game.gbc"));
    EXPECT_EQ(entry.data.size(), static_cast<qsizetype>(rom.size()));
}

TEST(RomArchiveTest, IgnoresNonRomEntries)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("mixed.zip"));
    const std::vector<uint8_t> rom = makeRom();
    ASSERT_TRUE(writeTestArchive(path.toStdString(), {
        {"readme.txt", {}},
        {"images/cover.png", {0x89, 'P', 'N', 'G'}},
        {"roms/game.gb", rom},
    }));

    qnemu::RomArchiveEntry entry;
    QString error;
    ASSERT_TRUE(qnemu::RomArchive::read(path, &entry, &error)) << error.toStdString();
    EXPECT_EQ(entry.name, QStringLiteral("roms/game.gb"));
}

TEST(RomArchiveTest, RejectsMultipleRoms)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("multi.zip"));
    const std::vector<uint8_t> rom = makeRom();
    ASSERT_TRUE(writeTestArchive(path.toStdString(), {
        {"a.gb", rom},
        {"b.gbc", rom},
    }));

    qnemu::RomArchiveEntry entry;
    QString error;
    EXPECT_FALSE(qnemu::RomArchive::read(path, &entry, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("more than one")));
}

TEST(RomArchiveTest, RejectsArchiveWithoutRom)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("empty.zip"));
    ASSERT_TRUE(writeTestArchive(path.toStdString(), {
        {"readme.txt", {}},
    }));

    qnemu::RomArchiveEntry entry;
    QString error;
    EXPECT_FALSE(qnemu::RomArchive::read(path, &entry, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("no .gb")));
}

TEST(RomArchiveTest, RejectsCorruptArchive)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = writeFile(directory.filePath(QStringLiteral("broken.zip")),
        QByteArrayLiteral("this is not a zip archive"));
    ASSERT_FALSE(path.isEmpty());

    qnemu::RomArchiveEntry entry;
    QString error;
    EXPECT_FALSE(qnemu::RomArchive::read(path, &entry, &error));
    EXPECT_FALSE(error.isEmpty());
}

TEST(RomArchiveTest, RejectsOversizedEntry)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("large.zip"));
    const std::vector<uint8_t> rom(
        static_cast<std::size_t>(qnemu::RomArchive::MaxRomSize) + 1, 0);
    ASSERT_TRUE(writeTestArchive(path.toStdString(), {{"large.gb", rom}}));

    qnemu::RomArchiveEntry entry;
    QString error;
    EXPECT_FALSE(qnemu::RomArchive::read(path, &entry, &error));
    EXPECT_TRUE(error.contains(QStringLiteral("too large")));
}

}  // namespace qnemuTest
