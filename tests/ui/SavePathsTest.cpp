/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <string>

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryDir>

#include <gtest/gtest.h>

#include "qnemu/save/SaveManager.h"
#include "qnemu/ui/SavePaths.h"

namespace qnemuTest
{

namespace
{

bool writeBytes(const QString& path, const QByteArray& data)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly)
        && file.write(data) == data.size();
}

QByteArray readBytes(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

}  // namespace
TEST(SavePathsTest, MatchesSaveManagerFileName)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString romPath = directory.filePath(QStringLiteral("Pokemon Red.gb"));
    const QString expected =
        QDir::cleanPath(QDir(directory.path()).filePath(QStringLiteral("Pokemon Red.sav")));
    EXPECT_EQ(qnemu::savePathForRom(directory.path(), romPath), expected);

    qnemu::SaveManager manager(directory.path().toStdString(), romPath.toStdString());
    EXPECT_EQ(QString::fromStdString(manager.ramPath()),
        qnemu::savePathForRom(directory.path(), romPath));
    EXPECT_EQ(QString::fromStdString(manager.rtcPath()),
        qnemu::rtcPathForRom(directory.path(), romPath));
}

TEST(SavePathsTest, UsesArchiveNameAndKeepsInnerDots)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    EXPECT_EQ(qnemu::saveFileStem(QStringLiteral("/roms/Tetris (World).zip")),
        QStringLiteral("Tetris (World)"));
    EXPECT_EQ(qnemu::saveFileStem(QStringLiteral("/roms/Game.v1.0.gb")),
        QStringLiteral("Game.v1.0"));
    EXPECT_EQ(qnemu::saveFileStem(QStringLiteral("/roms/no-extension")),
        QStringLiteral("no-extension"));

    const QString archivePath = directory.filePath(QStringLiteral("Tetris.zip"));
    const QString advertised = qnemu::savePathForRom(directory.path(), archivePath);
    qnemu::SaveManager manager(directory.path().toStdString(), archivePath.toStdString());
    EXPECT_EQ(advertised, QString::fromStdString(manager.ramPath()));
}

TEST(SavePathsTest, FallsBackWhenStemIsEmpty)
{
    EXPECT_EQ(qnemu::saveFileStem(QString()), QStringLiteral("save"));
    EXPECT_EQ(qnemu::savePathForRom(QStringLiteral("/saves"), QString()),
        QStringLiteral("/saves/save.sav"));
}

TEST(SavePathsTest, DetectsSameStemAcrossFoldersAndSeparatesLinkSlots)
{
    // Two different ROMs with the same file name share one normal save.
    EXPECT_EQ(qnemu::savePathForRom(QStringLiteral("/saves"),
                  QStringLiteral("/roms/a/Red.gb")),
        qnemu::savePathForRom(QStringLiteral("/saves"),
            QStringLiteral("/roms/b/Red.gb")));

    // A link slot gives the second player a separate persistent file.
    EXPECT_NE(qnemu::savePathForRom(QStringLiteral("/saves/link2"),
                  QStringLiteral("/roms/a/Red.gb")),
        qnemu::savePathForRom(QStringLiteral("/saves"),
            QStringLiteral("/roms/b/Red.gb")));
    EXPECT_EQ(qnemu::savePathForRom(QStringLiteral("/saves/link2"),
                  QStringLiteral("/roms/a/Red.gb")),
        QStringLiteral("/saves/link2/Red.sav"));
}

TEST(SavePathsTest, CopiesSaveAndRtcTogether)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString sourceSav = directory.filePath(QStringLiteral("source.sav"));
    const QString sourceRtc = directory.filePath(QStringLiteral("source.rtc"));
    const QString targetSav = directory.filePath(QStringLiteral("target.sav"));
    const QString targetRtc = directory.filePath(QStringLiteral("target.rtc"));
    ASSERT_TRUE(writeBytes(sourceSav, QByteArray("save data")));
    ASSERT_TRUE(writeBytes(sourceRtc, QByteArray("rtc data")));
    ASSERT_TRUE(writeBytes(targetSav, QByteArray("old save")));

    QString error;
    ASSERT_TRUE(qnemu::copySaveFiles(sourceSav, sourceRtc, targetSav, targetRtc, error))
        << error.toStdString();
    EXPECT_EQ(readBytes(targetSav), QByteArray("save data"));
    EXPECT_EQ(readBytes(targetRtc), QByteArray("rtc data"));
}

TEST(SavePathsTest, CopyWithoutRtcRemovesStaleTargetRtc)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString sourceSav = directory.filePath(QStringLiteral("source.sav"));
    const QString sourceRtc = directory.filePath(QStringLiteral("source.rtc"));
    const QString targetSav = directory.filePath(QStringLiteral("target.sav"));
    const QString targetRtc = directory.filePath(QStringLiteral("target.rtc"));
    ASSERT_TRUE(writeBytes(sourceSav, QByteArray("save data")));
    ASSERT_TRUE(writeBytes(targetRtc, QByteArray("stale rtc data")));

    QString error;
    ASSERT_TRUE(qnemu::copySaveFiles(sourceSav, sourceRtc, targetSav, targetRtc, error))
        << error.toStdString();
    EXPECT_EQ(readBytes(targetSav), QByteArray("save data"));
    EXPECT_FALSE(QFileInfo::exists(targetRtc));
}

TEST(SavePathsTest, CopyRejectsMissingSourceAndSamePath)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    QString error;
    const QString missing = directory.filePath(QStringLiteral("missing.sav"));
    const QString target = directory.filePath(QStringLiteral("target.sav"));
    EXPECT_FALSE(qnemu::copySaveFiles(missing, QString(), target, QString(), error));
    EXPECT_FALSE(error.isEmpty());

    ASSERT_TRUE(writeBytes(target, QByteArray("save")));
    error.clear();
    EXPECT_FALSE(qnemu::copySaveFiles(target, QString(), target, QString(), error));
    EXPECT_FALSE(error.isEmpty());
    EXPECT_EQ(readBytes(target), QByteArray("save"));
}

}  // namespace qnemuTest
