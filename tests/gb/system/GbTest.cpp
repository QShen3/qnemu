/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>
#include <utility>

#include <gtest/gtest.h>

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include "qnemu/gb/Gb.h"
#include "qnemu/input/InputInterface.h"
#include "tests/support/MemoryDisplay.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

namespace
{

class RecordingInput : public qnemu::InputInterface
{
public:
    void setButtonPressCallback(std::function<void(qnemu::GbButton)> callback) override
    {
        press = std::move(callback);
    }

    void setButtonReleaseCallback(std::function<void(qnemu::GbButton)> callback) override
    {
        release = std::move(callback);
    }

    std::function<void(qnemu::GbButton)> press;
    std::function<void(qnemu::GbButton)> release;
};

QString writeRom(const QTemporaryDir& directory, const QString& name, uint8_t cgbFlag)
{
    const std::array<uint8_t, 48> logo = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
        0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
        0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
        0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
        0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
        0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    };

    QByteArray data(0x8000, 0);
    std::copy(logo.begin(), logo.end(), data.begin() + 0x104);
    data.replace(0x134, 4, QByteArray("TEST"));
    data[0x143] = static_cast<char>(cgbFlag);
    data[0x147] = 0;
    data[0x148] = 0;  // 32 KiB ROM
    data[0x149] = 0;  // no cartridge RAM
    data[0x14A] = 1;
    data[0x14B] = 0x33;

    uint8_t checksum = 0;
    for (int address = 0x134; address <= 0x14C; ++address) {
        checksum = static_cast<uint8_t>(checksum - data.at(address) - 1);
    }
    data[0x14D] = static_cast<char>(checksum);
    if (cgbFlag == 0xC0) {
        uint16_t global = 0;
        for (int address = 0; address < data.size(); ++address) {
            if (address != 0x14E && address != 0x14F) {
                global = static_cast<uint16_t>(global + static_cast<uint8_t>(data.at(address)));
            }
        }
        data[0x14E] = static_cast<char>(global >> 8);
        data[0x14F] = static_cast<char>(global & 0xFF);
    }

    const QString path = directory.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size()) {
        return {};
    }
    return path;
}

}  // namespace

TEST(GbTest, ConstructsWithInjectedHostInterfaces)
{
    auto display = std::make_shared<MemoryDisplay>();
    qnemu::Gb system(qnemu::GbModel::dmg, display,
        std::make_unique<NullSound>(), nullptr);

    system.stop();
    EXPECT_EQ(display->frameCount, 0U);
}

TEST(GbTest, DualModeCartridgeUsesCgbHardwareWhenAllowed)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = writeRom(directory, QStringLiteral("dual.gbc"), 0x80);
    ASSERT_FALSE(path.isEmpty());

    auto display = std::make_shared<MemoryDisplay>();
    qnemu::Gb system(qnemu::GbModel::cgb, display,
        std::make_unique<NullSound>(), nullptr);
    system.loadCartridge(path.toStdString().c_str());
    EXPECT_TRUE(system.isCgbMode());
    system.stop();
}

TEST(GbTest, DmgOnlyCartridgeUsesCompatibilityPath)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = writeRom(directory, QStringLiteral("dmg.gb"), 0x00);
    ASSERT_FALSE(path.isEmpty());

    auto display = std::make_shared<MemoryDisplay>();
    qnemu::Gb system(qnemu::GbModel::cgb, display,
        std::make_unique<NullSound>(), nullptr);
    system.loadCartridge(path.toStdString().c_str());
    EXPECT_FALSE(system.isCgbMode());
    system.stop();
}

TEST(GbTest, DualModeCartridgeStaysDmgWhenDmgForced)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = writeRom(directory, QStringLiteral("dual.gb"), 0x80);
    ASSERT_FALSE(path.isEmpty());

    auto display = std::make_shared<MemoryDisplay>();
    qnemu::Gb system(qnemu::GbModel::dmg, display,
        std::make_unique<NullSound>(), nullptr);
    system.loadCartridge(path.toStdString().c_str());
    EXPECT_FALSE(system.isCgbMode());
    system.stop();
}

TEST(GbTest, CgbBootRomPresentsQnemuLogo)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    const QString path = writeRom(directory, QStringLiteral("boot.gbc"), 0xC0);
    ASSERT_FALSE(path.isEmpty());

    auto display = std::make_shared<MemoryDisplay>();
    qnemu::Gb system(qnemu::GbModel::cgb, display,
        std::make_unique<NullSound>(), nullptr);
    system.setBootRomEnabled(true);
    system.loadCartridge(path.toStdString().c_str());

    for (int i = 0; i < 120 && display->frameCount < 70; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    system.stop();

    ASSERT_GE(display->frameCount, 70U);
    const auto& frame = display->lastFrame;
    // The Q in the QNEMU wordmark is at x=34.., y=49..; its top bar is black.
    EXPECT_EQ(frame.at(37, 50), qnemu::rgb(0, 0, 0));
    EXPECT_EQ(frame.at(0, 0), qnemu::rgb(255, 255, 255));
}

TEST(GbTest, WiresAndClearsInputCallbacks)
{
    auto display = std::make_shared<MemoryDisplay>();
    auto input = std::make_shared<RecordingInput>();

    {
        qnemu::Gb system(qnemu::GbModel::dmg, display,
            std::make_unique<NullSound>(), input);
        EXPECT_TRUE(input->press);
        EXPECT_TRUE(input->release);
    }

    EXPECT_FALSE(input->press);
    EXPECT_FALSE(input->release);
}

}  // namespace qnemuTest
