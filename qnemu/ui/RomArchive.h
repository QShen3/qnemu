/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QByteArray>
#include <QString>
#include <QtGlobal>

namespace qnemu
{

struct RomArchiveEntry
{
    QString name;
    QByteArray data;
};

/**
 * Reads a single Game Boy ROM out of an archive.
 *
 * libarchive handles the container and compression method, so zip/7z/rar all
 * go through the same path.  Archives with zero or multiple ROM entries are
 * rejected instead of guessing which file the user meant.
 */
class RomArchive
{
public:
    /// GB/GBC ROMs are at most 8 MiB; leave room for unusual dumps.
    static constexpr qint64 MaxRomSize = 16 * 1024 * 1024;

    static bool isArchivePath(const QString& path);
    static bool read(const QString& path, RomArchiveEntry* entry,
        QString* error = nullptr);
};

}  // namespace qnemu
