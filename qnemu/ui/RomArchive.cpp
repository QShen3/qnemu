/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <memory>
#include <utility>

#include <archive.h>
#include <archive_entry.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>

#include "qnemu/ui/RomArchive.h"

namespace qnemu
{

namespace
{

constexpr qsizetype ReadBufferSize = 64 * 1024;

struct ArchiveSource
{
    QFile file;
    std::array<char, ReadBufferSize> buffer {};
};

int openArchive(archive*, void* data)
{
    auto* source = static_cast<ArchiveSource*>(data);
    if (!source->file.open(QIODevice::ReadOnly)) {
        return ARCHIVE_FATAL;
    }
    return ARCHIVE_OK;
}

la_ssize_t readArchive(archive*, void* data, const void** buffer)
{
    auto* source = static_cast<ArchiveSource*>(data);
    const qint64 read = source->file.read(
        source->buffer.data(), static_cast<qint64>(source->buffer.size()));
    if (read < 0) {
        return -1;
    }
    if (read == 0) {
        return 0;
    }
    *buffer = source->buffer.data();
    return static_cast<la_ssize_t>(read);
}

int closeArchive(archive*, void* data)
{
    auto* source = static_cast<ArchiveSource*>(data);
    source->file.close();
    return ARCHIVE_OK;
}

bool isRomEntry(const QString& name)
{
    if (name.startsWith(QStringLiteral("__MACOSX/"))) {
        return false;
    }
    const QString fileName = QFileInfo(name).fileName();
    if (fileName.startsWith(QStringLiteral("._"))) {
        return false;
    }
    const QString lower = fileName.toLower();
    return lower.endsWith(QStringLiteral(".gb"))
        || lower.endsWith(QStringLiteral(".gbc"))
        || lower.endsWith(QStringLiteral(".sgb"));
}

QString archiveError(archive* handle)
{
    const char* message = archive_error_string(handle);
    return message != nullptr ? QString::fromUtf8(message)
                              : QStringLiteral("unknown archive error");
}

}  // namespace

bool RomArchive::isArchivePath(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("zip") || suffix == QStringLiteral("7z")
        || suffix == QStringLiteral("rar");
}

bool RomArchive::read(const QString& path, RomArchiveEntry* entry, QString* error)
{
    if (entry != nullptr) {
        entry->name.clear();
        entry->data.clear();
    }
    if (error != nullptr) {
        error->clear();
    }
    if (entry == nullptr) {
        if (error != nullptr) {
            *error = QStringLiteral("No archive entry destination");
        }
        return false;
    }

    ArchiveSource source;
    source.file.setFileName(path);

    // `source` must outlive `handle`: archive_read_free may call the close
    // callback, and the callback receives &source.
    using ArchivePtr = std::unique_ptr<archive, decltype(&archive_read_free)>;
    ArchivePtr handle(archive_read_new(), archive_read_free);
    if (!handle) {
        if (error != nullptr) {
            *error = QStringLiteral("Cannot allocate archive reader");
        }
        return false;
    }

    if (archive_read_support_filter_all(handle.get()) < ARCHIVE_WARN
        || archive_read_support_format_all(handle.get()) < ARCHIVE_WARN) {
        if (error != nullptr) {
            *error = QStringLiteral("Cannot initialize archive support: %1")
                .arg(archiveError(handle.get()));
        }
        return false;
    }

    if (archive_read_open(handle.get(), &source, openArchive, readArchive,
            closeArchive) != ARCHIVE_OK) {
        if (error != nullptr) {
            *error = QStringLiteral("Cannot open archive %1: %2")
                .arg(path, archiveError(handle.get()));
        }
        return false;
    }

    bool found = false;
    archive_entry* rawEntry = nullptr;
    int result = ARCHIVE_OK;
    while ((result = archive_read_next_header(handle.get(), &rawEntry)) == ARCHIVE_OK) {
        const char* rawName = archive_entry_pathname_utf8(rawEntry);
        if (rawName == nullptr) {
            rawName = archive_entry_pathname(rawEntry);
        }
        const QString name = rawName != nullptr ? QString::fromUtf8(rawName) : QString();
        if (!isRomEntry(name)) {
            archive_read_data_skip(handle.get());
            continue;
        }

        if (found) {
            if (error != nullptr) {
                *error = QStringLiteral("Archive contains more than one Game Boy ROM");
            }
            return false;
        }

        const auto filetype = archive_entry_filetype(rawEntry);
        if (filetype != 0 && filetype != AE_IFREG) {
            archive_read_data_skip(handle.get());
            continue;
        }
        if (archive_entry_is_encrypted(rawEntry) != 0) {
            if (error != nullptr) {
                *error = QStringLiteral("Archive entry is encrypted: %1").arg(name);
            }
            return false;
        }

        const la_int64_t declaredSize = archive_entry_size(rawEntry);
        if (declaredSize > MaxRomSize) {
            if (error != nullptr) {
                *error = QStringLiteral("Archive entry is too large: %1").arg(name);
            }
            return false;
        }

        QByteArray data;
        if (declaredSize > 0) {
            data.reserve(static_cast<qsizetype>(declaredSize));
        }

        std::array<char, ReadBufferSize> buffer {};
        la_ssize_t read = 0;
        while ((read = archive_read_data(handle.get(), buffer.data(), buffer.size())) > 0) {
            if (static_cast<qint64>(data.size()) + read > MaxRomSize) {
                if (error != nullptr) {
                    *error = QStringLiteral("Archive entry is too large: %1").arg(name);
                }
                return false;
            }
            data.append(buffer.data(), static_cast<qsizetype>(read));
        }
        if (read < 0) {
            if (error != nullptr) {
                *error = QStringLiteral("Cannot read archive entry %1: %2")
                    .arg(name, archiveError(handle.get()));
            }
            return false;
        }

        entry->name = name;
        entry->data = std::move(data);
        found = true;
    }

    if (result != ARCHIVE_EOF) {
        if (error != nullptr) {
            *error = QStringLiteral("Cannot read archive %1: %2")
                .arg(path, archiveError(handle.get()));
        }
        return false;
    }
    if (!found) {
        if (error != nullptr) {
            *error = QStringLiteral("Archive contains no .gb, .gbc or .sgb file");
        }
        return false;
    }

    archive_read_close(handle.get());
    return true;
}

}  // namespace qnemu
