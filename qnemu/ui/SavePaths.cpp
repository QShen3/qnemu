/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/ui/SavePaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>

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

}  // namespace qnemu
