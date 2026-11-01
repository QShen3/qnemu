/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/ui/SavePaths.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>

namespace qnemu
{

QString saveFileStem(const QString& romPath)
{
    const QString stem = QFileInfo(romPath).completeBaseName();
    return stem.isEmpty() ? QStringLiteral("save") : stem;
}

QString savePathForRom(const QString& saveDirectory, const QString& romPath)
{
    return QDir::cleanPath(QDir(saveDirectory)
        .filePath(saveFileStem(romPath) + QStringLiteral(".sav")));
}

QString rtcPathForRom(const QString& saveDirectory, const QString& romPath)
{
    QString path = savePathForRom(saveDirectory, romPath);
    path.chop(4);  // ".sav"
    return path + QStringLiteral(".rtc");
}

bool copySaveFiles(const QString& sourceSav, const QString& sourceRtc,
    const QString& targetSav, const QString& targetRtc, QString& error)
{
    if (!QFileInfo::exists(sourceSav)) {
        error = QObject::tr("the source save does not exist");
        return false;
    }
    if (QFileInfo(sourceSav).absoluteFilePath()
        == QFileInfo(targetSav).absoluteFilePath()) {
        error = QObject::tr("the source and target are the same file");
        return false;
    }
    if (!QDir().mkpath(QFileInfo(targetSav).absolutePath())) {
        error = QObject::tr("could not create the save directory");
        return false;
    }

    if (QFileInfo::exists(targetSav)) {
        QFile target(targetSav);
        if (!target.remove()) {
            error = target.errorString();
            return false;
        }
    }
    QFile source(sourceSav);
    if (!source.copy(targetSav)) {
        error = source.errorString();
        return false;
    }

    // Keep .rtc in sync with .sav.  A missing source RTC removes a stale target
    // RTC instead of leaving the wrong wall-clock state behind.
    if (QFileInfo::exists(sourceRtc)) {
        if (QFileInfo::exists(targetRtc)) {
            QFile target(targetRtc);
            if (!target.remove()) {
                error = target.errorString();
                return false;
            }
        }
        QFile rtc(sourceRtc);
        if (!rtc.copy(targetRtc)) {
            error = rtc.errorString();
            return false;
        }
    } else if (QFileInfo::exists(targetRtc)) {
        QFile target(targetRtc);
        if (!target.remove()) {
            error = target.errorString();
            return false;
        }
    }
    return true;
}

QVector<SaveBackup> findSaveBackups(const QString& saveDirectory,
    const QString& romPath)
{    const QDir backupDirectory(QDir(saveDirectory).filePath(QStringLiteral("backups")));
    if (!backupDirectory.exists()) {
        return {};
    }

    const QString pattern = QStringLiteral(
        "^%1-(game|link1|link2|link3)-(\\d{8})-(\\d{6})-(\\d{3})\\.sav$")
        .arg(QRegularExpression::escape(saveFileStem(romPath)));
    const QRegularExpression regex(pattern);

    QVector<SaveBackup> backups;
    const QFileInfoList files = backupDirectory.entryInfoList(QDir::Files, QDir::Name);
    for (const QFileInfo& info : files) {
        const QRegularExpressionMatch match = regex.match(info.fileName());
        if (!match.hasMatch()) {
            continue;
        }

        SaveBackup backup;
        const QString slot = match.captured(1);
        backup.slot = slot == QStringLiteral("game")
            ? 0
            : (slot == QStringLiteral("link1")
                      ? 1
                      : (slot == QStringLiteral("link2") ? 2 : 3));
        backup.path = info.absoluteFilePath();
        backup.rtcPath = info.absolutePath() + QLatin1Char('/')
            + info.completeBaseName() + QStringLiteral(".rtc");

        QDateTime timestamp = QDateTime::fromString(
            match.captured(2) + match.captured(3),
            QStringLiteral("yyyyMMddHHmmss"));
        if (timestamp.isValid()) {
            timestamp = timestamp.addMSecs(match.captured(4).toInt());
        }
        backup.timestamp = timestamp;
        backups.append(backup);
    }

    std::sort(backups.begin(), backups.end(),
        [](const SaveBackup& left, const SaveBackup& right) {
            if (left.timestamp != right.timestamp) {
                return left.timestamp > right.timestamp;
            }
            return left.path > right.path;
        });
    return backups;
}

QString backupPathForSlot(const QString& saveDirectory, const QString& romPath,
    int slot, const QDateTime& timestamp)
{
    const QString suffix = slot == 0
        ? QStringLiteral("game")
        : (slot == 1
                  ? QStringLiteral("link1")
                  : (slot == 2 ? QStringLiteral("link2")
                               : QStringLiteral("link3")));
    const QString fileName = QStringLiteral("%1-%2-%3.sav")
        .arg(saveFileStem(romPath), suffix,
            timestamp.toString(QStringLiteral("yyyyMMdd-HHmmss-zzz")));
    return QDir::cleanPath(QDir(saveDirectory)
        .filePath(QStringLiteral("backups/") + fileName));
}

}  // namespace qnemu
