/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>


#include "qnemu/display/DisplayInterface.h"
#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/GbPlatform.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/infrared/GbInfraredLink.h"
#include "qnemu/gb/serial/GbSerialLink.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

class GameBoyPlatform;
class SaveManager;

/**
 * Complete Game Boy system.
 *
 * The host supplies the display, audio sink and optional input backend, so
 * this class and the `gb` library stay independent of Qt.  The display is
 * shared because the GUI keeps ownership of the window while the emulation
 * thread renders into it.
 */
class Gb
{
public:
    Gb(GbPlatform platformKind,
        std::shared_ptr<DisplayInterface> display,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input = nullptr,
        std::shared_ptr<GbSerialLink> serialLink = nullptr,
        std::shared_ptr<GbInfraredLink> infraredLink = nullptr);
    ~Gb();

    Gb(const Gb&) = delete;
    Gb& operator=(const Gb&) = delete;

    void loadCartridge(const char* filePath);
    /// Load a ROM image already in memory.  `saveIdentity` is used for the
    /// save-file stem; for archives this is the archive path.
    void loadCartridge(const std::vector<uint8_t>& buffer,
        const std::string& saveIdentity);
    void stop();

    /// True when the loaded cartridge is running through the CGB hardware path.
    bool isCgbMode() const;
    void setPaused(bool paused);
    bool isPaused() const { return paused.load(); }

    void setSaveDirectory(std::string directory) { saveDirectory = std::move(directory); }
    void setSoundEnabled(bool enabled);

    const FrameBuffer& frameBuffer() const;

    /// Native output size: 160x144 for GB/GBC, 256x224 for SGB.
    std::pair<int, int> frameSize() const;

private:
    void prepareCartridgeLoad();
    void finishCartridgeLoad(const std::string& saveIdentity);
    void start();
    void run();

    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::unique_ptr<GameBoyPlatform> platform;
    SoundInterface* sound = nullptr;
    const GbMbcFactory mbcFactory;
    std::shared_ptr<DisplayInterface> display;
    std::shared_ptr<InputInterface> input;
    std::unique_ptr<SaveManager> saveManager;
    std::string saveDirectory;
    std::thread work;
    std::atomic_bool running{false};
    std::atomic_bool paused{false};
    std::mutex pauseMutex;
    std::condition_variable pauseCondition;
};

}  // namespace qnemu
