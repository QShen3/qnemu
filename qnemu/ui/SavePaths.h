/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QString>

namespace qnemu
{

/// Stem used for a ROM's save files, matching SaveManager's naming.
QString saveFileStem(const QString& romPath);

/// Full path of the .sav file for `romPath` inside `saveDirectory`.
QString savePathForRom(const QString& saveDirectory, const QString& romPath);

/// Full path of the matching .rtc file (same stem, .rtc extension).
QString rtcPathForRom(const QString& saveDirectory, const QString& romPath);

/// Copies a .sav and its optional .rtc.  When the source RTC does not exist,
/// any stale target RTC is removed.  On failure `error` describes the reason.
bool copySaveFiles(const QString& sourceSav, const QString& sourceRtc,
    const QString& targetSav, const QString& targetRtc, QString& error);

}  // namespace qnemu
