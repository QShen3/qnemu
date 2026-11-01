/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <chrono>
#include <memory>
#include <thread>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include <gtest/gtest.h>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/serial/GbFourPlayerAdapter.h"
#include "tests/support/MemoryDisplay.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

TEST(GbFourPlayerLinkTest, FourSystemsExchangePlayerIdsThroughTheAdapter)
{
#ifndef QNEMU_FOUR_PLAYER_TEST_ROM
    GTEST_SKIP() << "four-player test ROM is not configured";
#else
    const QString romPath = QStringLiteral(QNEMU_FOUR_PLAYER_TEST_ROM);
    if (!QFile::exists(romPath)) {
        GTEST_SKIP() << "four-player test ROM is missing";
    }

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const auto links = qnemu::createFourPlayerAdapter(4);
    ASSERT_EQ(links.size(), 4U);

    std::array<std::shared_ptr<MemoryDisplay>, 4> displays;
    std::array<std::unique_ptr<qnemu::Gb>, 4> systems;
    std::array<QString, 4> saveDirectories;

    for (int player = 0; player < 4; ++player) {
        saveDirectories.at(player) = directory.filePath(
            QStringLiteral("saves%1").arg(player + 1));
        ASSERT_TRUE(QDir().mkpath(saveDirectories.at(player)));

        displays.at(player) = std::make_shared<MemoryDisplay>();
        systems.at(player) = std::make_unique<qnemu::Gb>(
            qnemu::GbPlatform::gb, displays.at(player),
            std::make_unique<NullSound>(), nullptr, links.at(player));
        systems.at(player)->setSaveDirectory(
            saveDirectories.at(player).toStdString());
        systems.at(player)->loadCartridge(romPath.toStdString().c_str());
    }

    // The ROM reports success by writing 1 to SRAM; the save is flushed when
    // the systems are destroyed.
    std::this_thread::sleep_for(std::chrono::seconds(3));

    for (auto& system : systems) {
        system.reset();
    }

    for (int player = 0; player < 4; ++player) {
        QFile save(QDir(saveDirectories.at(player))
                       .filePath(QStringLiteral("four_player_test.sav")));
        ASSERT_TRUE(save.open(QIODevice::ReadOnly))
            << "player " << player << " did not write a save";
        const QByteArray data = save.readAll();
        ASSERT_FALSE(data.isEmpty());
        EXPECT_EQ(static_cast<uint8_t>(data.at(0)), 1)
            << "player " << player << " did not pass the four-player test";
    }
#endif
}

}  // namespace qnemuTest
