/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/platform/GameBoyPlatform.h"
#include "qnemu/gb/platform/PlatformFactory.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/save/SaveManager.h"

namespace qnemu
{

Gb::Gb(GbPlatform platformKind,
        std::shared_ptr<DisplayInterface> displayInterface,
        std::unique_ptr<SoundInterface> soundInterface,
        std::shared_ptr<InputInterface> inputInterface,
        std::shared_ptr<GbSerialLink> serialLink,
        std::shared_ptr<GbInfraredLink> infraredLink) :
    display(std::move(displayInterface)),
    input(std::move(inputInterface))
{
    sound = soundInterface.get();
    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    platform = createPlatform(platformKind, cartridge, display, std::move(soundInterface),
        input, std::move(serialLink), std::move(infraredLink));
}

Gb::~Gb()
{
    stop();
    if (saveManager && cartridge) {
        saveManager->save(*cartridge);
    }
    saveManager.reset();

    // The platform wires the host input callbacks to its joypad.  Tear it
    // down before clearing the callbacks so the captures are always valid.
    platform.reset();
    if (input) {
        input->setButtonPressCallback({});
        input->setButtonReleaseCallback({});
    }
}

void Gb::prepareCartridgeLoad()
{
    stop();
    if (saveManager && cartridge) {
        saveManager->save(*cartridge);
    }
    saveManager.reset();
}

void Gb::loadCartridge(const char* filePath)
{
    prepareCartridgeLoad();
    cartridge->load(filePath);
    finishCartridgeLoad(filePath);
}

void Gb::loadCartridge(const std::vector<uint8_t>& buffer,
    const std::string& saveIdentity)
{
    prepareCartridgeLoad();
    cartridge->load(buffer);
    finishCartridgeLoad(saveIdentity);
}

void Gb::finishCartridgeLoad(const std::string& saveIdentity)
{
    // The platform owns its boot ROM and reset sequence.  CGB hardware starts
    // in CGB mode even for DMG-only cartridges; its boot ROM selects the
    // per-game compatibility palette and switches to DMG mode through KEY0.
    // DMG uses Bootix and SGB uses qnemu's clean-room header-forwarding ROM.
    platform->reset(cartridge->cartridgeHeader());

    saveManager = std::make_unique<SaveManager>(
        saveDirectory.empty() ? SaveManager::defaultDirectory() : saveDirectory, saveIdentity);
    saveManager->load(*cartridge);

    setPaused(false);
    start();
}

void Gb::start()
{
    if (!running.exchange(true)) {
        work = std::thread(&Gb::run, this);
    }
}

void Gb::stop()
{
    running.store(false);
    pauseCondition.notify_all();
    if (work.joinable()) {
        work.join();
    }
}

void Gb::setPaused(bool value)
{
    if (paused.exchange(value) == value) {
        return;
    }
    if (value) {
        if (sound) {
            sound->flush();
        }
    } else {
        pauseCondition.notify_all();
    }
}

void Gb::setSoundEnabled(bool enabled)
{
    if (!sound) {
        return;
    }
    if (enabled) {
        sound->enable();
    } else {
        sound->disable();
    }
}

bool Gb::isCgbMode() const
{
    return platform && platform->isCgbMode();
}

const FrameBuffer& Gb::frameBuffer() const
{
    return platform->frameBuffer();
}

std::pair<int, int> Gb::frameSize() const
{
    return platform ? platform->frameSize() : std::pair {160, 144};
}

void Gb::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr auto frameInterval = std::chrono::microseconds(16743);

    bool displayEnabled = true;
    uint64_t lastFrameCount = platform ? platform->frameCount() : 0;
    auto nextFrame = Clock::now();

    while (running.load()) {
        if (paused.load()) {
            std::unique_lock lock(pauseMutex);
            pauseCondition.wait(lock, [this] {
                return !paused.load() || !running.load();
            });
        }
        if (!running.load()) {
            break;
        }

        platform->step();

        const bool stopped = platform->isCpuStopped();
        if (display) {
            if (stopped && displayEnabled) {
                display->disable();
                displayEnabled = false;
            } else if (!stopped && !displayEnabled) {
                display->enable();
                displayEnabled = true;
            }
        }

        const uint64_t frameCount = platform ? platform->frameCount() : 0;
        if (frameCount != lastFrameCount) {
            lastFrameCount = frameCount;
            nextFrame += frameInterval;
            const auto now = Clock::now();
            if (nextFrame < now) {
                // Do not accumulate a large debt after a slow frame.
                nextFrame = now;
            } else {
                std::this_thread::sleep_until(nextFrame);
            }
        }
    }
}

}  // namespace qnemu
