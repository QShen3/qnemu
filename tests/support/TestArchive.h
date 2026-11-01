/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <archive.h>
#include <archive_entry.h>

namespace qnemuTest
{

struct TestArchiveEntry
{
    std::string name;
    std::vector<uint8_t> data;
};

/// Writes a small ZIP fixture for the archive-reader tests.
inline bool writeTestArchive(const std::string& path,
    const std::vector<TestArchiveEntry>& entries,
    const char* compression = "deflate")
{
    archive* handle = archive_write_new();
    if (handle == nullptr) {
        return false;
    }

    bool ok = archive_write_set_format_zip(handle) == ARCHIVE_OK;
    if (ok) {
        const std::string options = std::string("zip:compression=") + compression;
        ok = archive_write_set_options(handle, options.c_str()) == ARCHIVE_OK;
    }
    if (ok) {
        ok = archive_write_open_filename(handle, path.c_str()) == ARCHIVE_OK;
    }

    if (ok) {
        for (const TestArchiveEntry& item : entries) {
            archive_entry* entry = archive_entry_new();
            if (entry == nullptr) {
                ok = false;
                break;
            }
            archive_entry_set_pathname(entry, item.name.c_str());
            archive_entry_set_filetype(entry, AE_IFREG);
            archive_entry_set_perm(entry, 0644);
            archive_entry_set_size(entry, static_cast<la_int64_t>(item.data.size()));
            if (archive_write_header(handle, entry) != ARCHIVE_OK) {
                archive_entry_free(entry);
                ok = false;
                break;
            }
            if (!item.data.empty()) {
                const la_ssize_t written = archive_write_data(
                    handle, item.data.data(), item.data.size());
                if (written != static_cast<la_ssize_t>(item.data.size())) {
                    archive_entry_free(entry);
                    ok = false;
                    break;
                }
            }
            archive_entry_free(entry);
        }
    }

    if (ok) {
        ok = archive_write_close(handle) == ARCHIVE_OK;
    } else {
        archive_write_close(handle);
    }
    archive_write_free(handle);
    return ok;
}

}  // namespace qnemuTest
