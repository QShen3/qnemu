/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <QtCore/QByteArray>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSaveFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>

#include "qnemu/save/SaveManager.h"

namespace qnemu
{

namespace
{

constexpr std::array<uint8_t, 4> RtcMagic = {'Q', 'N', 'R', 'T'};
constexpr uint8_t RtcVersion = 1;
constexpr std::size_t RtcHeaderSize = RtcMagic.size() + 1 + 8;

uint64_t currentUnixTime()
{
    return static_cast<uint64_t>(std::time(nullptr));
}

std::string fileNameStem(const std::string& romPath)
{
    std::string stem = std::filesystem::path(romPath).stem().string();
    if (stem.empty()) {
        stem = "save";
    }
    return stem;
}

}  // namespace

SaveManager::SaveManager(std::string directory, std::string romPath)
{
    directoryPath = std::move(directory);
    const std::string stem = fileNameStem(romPath);
    ramFilePath = (std::filesystem::path(directoryPath) / (stem + ".sav")).string();
    rtcFilePath = (std::filesystem::path(directoryPath) / (stem + ".rtc")).string();
}

std::string SaveManager::defaultDirectory()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) {
        base = QDir::homePath() + "/.qnemu";
    }
    const QString directory = QDir(base).filePath("saves");
    QDir().mkpath(directory);
    return directory.toStdString();
}

bool SaveManager::load(SaveTarget& target)
{
    bool loaded = false;

    if (target.hasBattery() && target.ramSize() > 0) {
        QFile file(QString::fromStdString(ramFilePath));
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray bytes = file.readAll();
            std::vector<uint8_t> ram(target.ramSize(), 0xFF);
            const std::size_t count = std::min(
                ram.size(), static_cast<std::size_t>(bytes.size()));
            std::copy_n(reinterpret_cast<const uint8_t*>(bytes.constData()), count, ram.begin());
            target.loadRam(ram);
            loaded = true;
        }
    }

    if (target.hasRtc()) {
        QFile file(QString::fromStdString(rtcFilePath));
        if (file.open(QIODevice::ReadOnly)) {
            const QByteArray bytes = file.readAll();
            if (static_cast<std::size_t>(bytes.size()) >= RtcHeaderSize + 5
                && std::equal(RtcMagic.begin(), RtcMagic.end(),
                    reinterpret_cast<const uint8_t*>(bytes.constData()))
                && static_cast<uint8_t>(bytes.at(RtcMagic.size())) == RtcVersion) {
                const auto* data = reinterpret_cast<const uint8_t*>(bytes.constData());
                uint64_t savedTime = 0;
                for (std::size_t i = 0; i < 8; ++i) {
                    savedTime |= static_cast<uint64_t>(data[RtcMagic.size() + 1 + i]) << (8 * i);
                }

                std::vector<uint8_t> rtc(data + RtcHeaderSize, data + RtcHeaderSize + 5);
                target.loadRtc(rtc);

                const uint64_t now = currentUnixTime();
                if (now > savedTime) {
                    target.advanceRtc(now - savedTime);
                }
                loaded = true;
            }
        }
    }

    return loaded;
}

bool SaveManager::save(SaveTarget& target)
{
    if (!target.hasBattery() && !target.hasRtc()) {
        return false;
    }

    std::error_code error;
    std::filesystem::create_directories(directoryPath, error);
    if (error) {
        return false;
    }

    bool ok = true;

    if (target.hasBattery() && target.ramSize() > 0) {
        std::vector<uint8_t> ram = target.dumpRam();
        ram.resize(target.ramSize(), 0xFF);
        ok = writeFile(ramFilePath, ram) && ok;
    }

    if (target.hasRtc()) {
        const std::vector<uint8_t> rtc = target.dumpRtc();
        if (rtc.size() == 5) {
            std::vector<uint8_t> data;
            data.reserve(RtcHeaderSize + rtc.size());
            data.insert(data.end(), RtcMagic.begin(), RtcMagic.end());
            data.push_back(RtcVersion);

            const uint64_t now = currentUnixTime();
            for (std::size_t i = 0; i < 8; ++i) {
                data.push_back(static_cast<uint8_t>((now >> (8 * i)) & 0xFF));
            }
            data.insert(data.end(), rtc.begin(), rtc.end());
            ok = writeFile(rtcFilePath, data) && ok;
        } else {
            ok = false;
        }
    }

    return ok;
}

bool SaveManager::writeFile(const std::string& path, const std::vector<uint8_t>& data) const
{
    QSaveFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    if (file.write(reinterpret_cast<const char*>(data.data()),
            static_cast<qint64>(data.size())) != static_cast<qint64>(data.size())) {
        file.cancelWriting();
        return false;
    }
    return file.commit();
}

}  // namespace qnemu
