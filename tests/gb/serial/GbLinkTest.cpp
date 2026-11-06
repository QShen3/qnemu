/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <thread>
#include <vector>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include <gtest/gtest.h>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/serial/GbLocalSerialLink.h"
#include "tests/support/MemoryDisplay.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

namespace
{

constexpr std::array<uint8_t, 48> nintendoLogo = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

std::vector<uint8_t> makeLinkTestRom(bool master)
{
    std::vector<uint8_t> rom(0x8000, 0);

    // Entry point, logo and a minimal MBC1+RAM+battery header.
    rom[0x100] = 0x00;
    rom[0x101] = 0xC3;
    rom[0x102] = 0x50;
    rom[0x103] = 0x01;
    std::copy(nintendoLogo.begin(), nintendoLogo.end(), rom.begin() + 0x104);

    constexpr char title[] = "LINKTEST";
    std::fill(rom.begin() + 0x134, rom.begin() + 0x144, 0);
    std::copy(title, title + sizeof(title) - 1, rom.begin() + 0x134);
    rom[0x147] = 0x03;  // MBC1 + RAM + battery
    rom[0x148] = 0x00;  // 32 KiB ROM
    rom[0x149] = 0x02;  // 8 KiB RAM
    rom[0x14A] = 0x01;
    rom[0x14B] = 0x33;
    rom[0x14C] = 0x00;

    uint8_t headerChecksum = 0;
    for (int address = 0x134; address <= 0x14C; ++address) {
        headerChecksum = static_cast<uint8_t>(headerChecksum - rom.at(address) - 1);
    }
    rom[0x14D] = headerChecksum;
    uint16_t globalChecksum = 0;
    for (std::size_t address = 0; address < rom.size(); ++address) {
        if (address != 0x14E && address != 0x14F) {
            globalChecksum = static_cast<uint16_t>(
                globalChecksum + rom.at(address));
        }
    }
    rom[0x14E] = static_cast<uint8_t>(globalChecksum >> 8);
    rom[0x14F] = static_cast<uint8_t>(globalChecksum & 0xFF);

    std::size_t pc = 0x150;
    const auto emitBytes = [&rom, &pc](std::initializer_list<uint8_t> bytes) {
        for (const uint8_t byte : bytes) {
            rom.at(pc++) = byte;
        }
    };

    // Enable cartridge RAM.
    emitBytes({0x3E, 0x0A, 0xEA, 0x00, 0x00});

    if (master) {
        // Give the slave time to boot and arm its transfer.
        for (int loop = 0; loop < 2; ++loop) {
            emitBytes({0x01, 0xFF, 0xFF});  // ld bc, $FFFF
            const std::size_t target = pc;
            emitBytes({0x0B, 0x78, 0xB1});  // dec bc; ld a, b; or c
            emitBytes({0x20, static_cast<uint8_t>(target - (pc + 2))});
        }
    }

    emitBytes({0x3E, static_cast<uint8_t>(master ? 0xA5 : 0x5A)});
    emitBytes({0xE0, 0x01});  // SB
    emitBytes({0x3E, static_cast<uint8_t>(master ? 0x81 : 0x80)});
    emitBytes({0xE0, 0x02});  // SC

    const std::size_t waitTarget = pc;
    emitBytes({0xF0, 0x02, 0xE6, 0x80});  // ldh a, (SC); and $80
    emitBytes({0x20, static_cast<uint8_t>(waitTarget - (pc + 2))});

    emitBytes({0xF0, 0x01});  // ldh a, (SB)
    emitBytes({0xFE, static_cast<uint8_t>(master ? 0x5A : 0xA5)});

    const std::size_t failJump = pc;
    emitBytes({0x20, 0x00});
    emitBytes({0x3E, 0x01});  // ld a, 1 (pass)
    const std::size_t storeJump = pc;
    emitBytes({0x18, 0x00});
    const std::size_t failTarget = pc;
    emitBytes({0xAF});  // xor a (fail)
    const std::size_t storeTarget = pc;
    emitBytes({0xEA, 0x00, 0xA0});  // ld ($A000), a
    emitBytes({0x18, 0xFE});        // jr $

    rom.at(failJump + 1) =
        static_cast<uint8_t>(failTarget - (failJump + 2));
    rom.at(storeJump + 1) =
        static_cast<uint8_t>(storeTarget - (storeJump + 2));

    return rom;
}

bool writeRom(const QString& path, const std::vector<uint8_t>& rom)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(reinterpret_cast<const char*>(rom.data()),
               static_cast<qint64>(rom.size()))
        == static_cast<qint64>(rom.size());
}

}  // namespace

TEST(GbLinkTest, TwoSystemsExchangeBytesThroughTheLinkCable)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const QString masterRomPath = directory.filePath(QStringLiteral("master.gb"));
    const QString slaveRomPath = directory.filePath(QStringLiteral("slave.gb"));
    ASSERT_TRUE(writeRom(masterRomPath, makeLinkTestRom(true)));
    ASSERT_TRUE(writeRom(slaveRomPath, makeLinkTestRom(false)));

    const QString masterSaveDirectory = directory.filePath(QStringLiteral("saves1"));
    const QString slaveSaveDirectory = directory.filePath(QStringLiteral("saves2"));
    ASSERT_TRUE(QDir().mkpath(masterSaveDirectory));
    ASSERT_TRUE(QDir().mkpath(slaveSaveDirectory));

    auto masterDisplay = std::make_shared<MemoryDisplay>();
    auto slaveDisplay = std::make_shared<MemoryDisplay>();
    const auto [masterLink, slaveLink] = qnemu::createLocalSerialLinkPair();

    {
        qnemu::Gb master(qnemu::GbPlatform::gb, masterDisplay,
            std::make_unique<NullSound>(), nullptr, masterLink);
        qnemu::Gb slave(qnemu::GbPlatform::gb, slaveDisplay,
            std::make_unique<NullSound>(), nullptr, slaveLink);

        master.setSaveDirectory(masterSaveDirectory.toStdString());
        slave.setSaveDirectory(slaveSaveDirectory.toStdString());
        master.loadCartridge(masterRomPath.toStdString().c_str());
        slave.loadCartridge(slaveRomPath.toStdString().c_str());

        // The ROMs report their result through cartridge RAM, which is
        // flushed when the systems are destroyed.  Wait for emulated frames
        // instead of wall-clock time so coverage and debug builds reach the
        // same point as fast release builds.
        ASSERT_TRUE(masterDisplay->waitForFrames(300U));
        ASSERT_TRUE(slaveDisplay->waitForFrames(300U));
    }

    const auto readResult = [](const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return -1;
        }
        const QByteArray data = file.readAll();
        return data.isEmpty() ? -1 : static_cast<int>(
            static_cast<uint8_t>(data.at(0)));
    };

    EXPECT_EQ(readResult(QDir(masterSaveDirectory).filePath(QStringLiteral("master.sav"))), 1);
    EXPECT_EQ(readResult(QDir(slaveSaveDirectory).filePath(QStringLiteral("slave.sav"))), 1);
}

}  // namespace qnemuTest
