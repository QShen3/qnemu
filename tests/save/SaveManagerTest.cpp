/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>

#include <QTemporaryDir>
#include <gtest/gtest.h>

#include "qnemu/save/SaveManager.h"

namespace qnemuTest
{

namespace
{

class FakeSaveTarget : public qnemu::SaveTarget
{
public:
    explicit FakeSaveTarget(std::vector<uint8_t> initialRam = {1, 2, 3, 4}) :
        ram(std::move(initialRam))
    {
    }

    bool hasBattery() const override { return battery; }

    std::size_t ramSize() const override { return ram.size(); }

    std::vector<uint8_t> dumpRam() const override { return ram; }

    void loadRam(const std::vector<uint8_t>& data) override { ram = data; }

    bool hasRtc() const override { return rtc; }

    std::vector<uint8_t> dumpRtc() override { return rtcData; }

    void loadRtc(const std::vector<uint8_t>& data) override { rtcData = data; }

    void advanceRtc(uint64_t seconds) override { advanced += seconds; }

    bool battery = true;
    bool rtc = false;
    std::vector<uint8_t> ram;
    std::vector<uint8_t> rtcData {0x12, 0x34, 0x05, 0x10, 0x01};
    uint64_t advanced = 0;
};

}  // namespace

TEST(SaveManagerTest, RamRoundTripUsesRomStemInUnifiedDirectory)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    qnemu::SaveManager manager(
        directory.path().toStdString(), "/roms/Tetris (World).gb");
    FakeSaveTarget target;
    target.ram = {0x10, 0x20, 0x30, 0x40};

    EXPECT_TRUE(manager.save(target));
    EXPECT_EQ(std::filesystem::path(manager.ramPath()).filename(),
        "Tetris (World).sav");
    EXPECT_TRUE(std::filesystem::exists(manager.ramPath()));

    FakeSaveTarget loaded(std::vector<uint8_t>(4, 0));
    EXPECT_TRUE(manager.load(loaded));
    EXPECT_EQ(loaded.ram, target.ram);
}

TEST(SaveManagerTest, RtcRoundTripUsesSeparateFile)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    qnemu::SaveManager manager(directory.path().toStdString(), "/roms/Pokemon.gbc");
    FakeSaveTarget target;
    target.rtc = true;
    target.battery = false;

    EXPECT_TRUE(manager.save(target));
    EXPECT_TRUE(std::filesystem::exists(manager.rtcPath()));
    EXPECT_FALSE(std::filesystem::exists(manager.ramPath()));

    FakeSaveTarget loaded(std::vector<uint8_t> {});
    loaded.rtc = true;
    loaded.battery = false;
    EXPECT_TRUE(manager.load(loaded));
    EXPECT_EQ(loaded.rtcData, target.rtcData);
}

TEST(SaveManagerTest, NoBatteryAndNoRtcWritesNothing)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    qnemu::SaveManager manager(directory.path().toStdString(), "/roms/Plain.gb");
    FakeSaveTarget target;
    target.battery = false;

    EXPECT_FALSE(manager.save(target));
    EXPECT_TRUE(std::filesystem::is_empty(directory.path().toStdString()));
}

TEST(SaveManagerTest, MissingFilesLeaveTargetUnchanged)
{
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());

    qnemu::SaveManager manager(directory.path().toStdString(), "/roms/Tetris.gb");
    FakeSaveTarget target(std::vector<uint8_t> {0xAA, 0xBB});

    EXPECT_FALSE(manager.load(target));
    EXPECT_EQ(target.ram, std::vector<uint8_t>({0xAA, 0xBB}));
}

}  // namespace qnemuTest
