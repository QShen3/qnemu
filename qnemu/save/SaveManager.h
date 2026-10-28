/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "qnemu/save/SaveTarget.h"

namespace qnemu
{

/**
 * Saves raw cartridge RAM and RTC state in a per-user directory.
 *
 * File names are derived from the ROM stem:
 *   <stem>.sav   raw SRAM dump
 *   <stem>.rtc   MBC3 RTC registers + wall-clock timestamp
 *
 * The directory is injected so a future settings page can change it without
 * touching the save format.
 */
class SaveManager
{
public:
    SaveManager(std::string directory, std::string romPath);

    bool load(SaveTarget& target);
    bool save(SaveTarget& target);

    const std::string& directory() const { return directoryPath; }
    const std::string& ramPath() const { return ramFilePath; }
    const std::string& rtcPath() const { return rtcFilePath; }

    /// Default application-data save directory, created if necessary.
    static std::string defaultDirectory();

private:
    bool writeFile(const std::string& path, const std::vector<uint8_t>& data) const;

    std::string directoryPath;
    std::string ramFilePath;
    std::string rtcFilePath;
};

}  // namespace qnemu
