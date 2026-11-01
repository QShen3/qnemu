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
#include "qnemu/gb/infrared/GbLocalInfraredLink.h"
#include "tests/support/MemoryDisplay.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

TEST(GbInfraredLinkTest, TwoCgbSystemsExchangePulses)
{
#if !defined(QNEMU_IR_SENDER_ROM) || !defined(QNEMU_IR_RECEIVER_ROM)
    GTEST_SKIP() << "infrared test ROMs are not configured";
#else
    const QString senderRom = QStringLiteral(QNEMU_IR_SENDER_ROM);
    const QString receiverRom = QStringLiteral(QNEMU_IR_RECEIVER_ROM);
    if (!QFile::exists(senderRom) || !QFile::exists(receiverRom)) {
        GTEST_SKIP() << "infrared test ROMs are missing";
    }

    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    const auto [senderLink, receiverLink] = qnemu::createLocalInfraredLinkPair();
    const QString senderSaveDirectory =
        directory.filePath(QStringLiteral("sender-saves"));
    const QString receiverSaveDirectory =
        directory.filePath(QStringLiteral("receiver-saves"));
    ASSERT_TRUE(QDir().mkpath(senderSaveDirectory));
    ASSERT_TRUE(QDir().mkpath(receiverSaveDirectory));

    auto senderDisplay = std::make_shared<MemoryDisplay>();
    auto receiverDisplay = std::make_shared<MemoryDisplay>();

    {
        qnemu::Gb sender(qnemu::GbPlatform::gbc, senderDisplay,
            std::make_unique<NullSound>(), nullptr, nullptr, senderLink);
        qnemu::Gb receiver(qnemu::GbPlatform::gbc, receiverDisplay,
            std::make_unique<NullSound>(), nullptr, nullptr, receiverLink);

        sender.setSaveDirectory(senderSaveDirectory.toStdString());
        receiver.setSaveDirectory(receiverSaveDirectory.toStdString());
        sender.loadCartridge(senderRom.toStdString().c_str());
        receiver.loadCartridge(receiverRom.toStdString().c_str());

        std::this_thread::sleep_for(std::chrono::seconds(4));
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

    EXPECT_EQ(readResult(QDir(senderSaveDirectory)
                      .filePath(QStringLiteral("ir_sender.sav"))),
        1);
    EXPECT_EQ(readResult(QDir(receiverSaveDirectory)
                      .filePath(QStringLiteral("ir_receiver.sav"))),
        1);
#endif
}

}  // namespace qnemuTest
