/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "qnemu/gb/boot/GbCgbBootRom.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/CgbCompatibilityPalette.h"
#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbCpuBus.h"
#include "qnemu/gb/mmu/GbMmu.h"
#include "qnemu/gb/sgb/GbSgb.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "qnemu/save/SaveManager.h"

namespace qnemu
{

Gb::Gb(GbModel model,
        std::shared_ptr<DisplayInterface> displayInterface,
        std::unique_ptr<SoundInterface> soundInterface,
        std::shared_ptr<InputInterface> inputInterface) :
    model(model),
    display(std::move(displayInterface)),
    input(std::move(inputInterface))
{
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels {
        std::make_unique<GbChannel1>(),
        std::make_unique<GbChannel2>(),
        std::make_unique<GbChannel3>(),
        std::make_unique<GbChannel4>(),
    };
    sound = soundInterface.get();
    auto apu = std::make_unique<GbApu>(std::move(soundInterface), std::move(channels),
        model == GbModel::cgb, GbApu::DefaultFadeInFrames);

    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    auto interruptHandler = std::make_shared<GbInterruptHandler>();

    if (model == GbModel::sgb) {
        sgb = std::make_unique<GbSgb>();
        sgb->reset();
    }

    auto workRam = std::make_unique<GbWorkRam>();
    auto highRam = std::make_unique<GbHighRam>();
    auto timer = std::make_unique<GbTimer>(interruptHandler);

    auto joypad = std::make_unique<GbJoypad>(interruptHandler);
    if (sgb) {
        GbSgb* sgbPointer = sgb.get();
        joypad->setSgbWriteCallback([sgbPointer](uint8_t value) {
            sgbPointer->write(value);
        });
    }
    if (input) {
        auto* joypadPointer = joypad.get();
        input->setButtonPressCallback([joypadPointer](GbButton button) {
            joypadPointer->setButtonState(button, true);
        });
        input->setButtonReleaseCallback([joypadPointer](GbButton button) {
            joypadPointer->setButtonState(button, false);
        });
    }

    auto ppuInstance = std::make_unique<GbPpu>(
        model == GbModel::cgb && cartridge->isGbcCartridge(), display, interruptHandler);
    ppu = ppuInstance.get();
    ppu->setOamBugEnabled(model != GbModel::cgb);
    ppu->setSgb(sgb.get());

    mmu = std::make_unique<GbMmu>(std::move(apu), cartridge, std::move(ppuInstance), std::move(highRam),
        interruptHandler, std::move(joypad), std::move(workRam), std::move(timer));
    ppu->setExternalRead([mmuPointer = mmu.get()](uint16_t address) {
        return mmuPointer->read(address);
    });
    bus = std::make_unique<GbCpuBus>(
        *mmu, model == GbModel::cgb && cartridge->isGbcCartridge());
    bus->setCgbModeCallback([this](bool mode) {
        // The PPU stays on CGB hardware registers in DMG compatibility mode;
        // only its rendering mode switches to the DMG path.
        ppu->setDmgMode(!mode);
        if (mode) {
            ppu->clearDmgCompatibilityPalette();
        } else {
            ppu->enableDmgCompatibility();
        }
    });
    cpu = std::make_unique<GbCpu>(*bus);
}

Gb::~Gb()
{
    stop();
    if (saveManager && cartridge) {
        saveManager->save(*cartridge);
    }
    saveManager.reset();

    if (input) {
        // The input callbacks capture the joypad's address; drop them before
        // the MMU (which owns the joypad) is destroyed.
        input->setButtonPressCallback({});
        input->setButtonReleaseCallback({});
    }
}

void Gb::loadCartridge(const char* filePath)
{
    stop();
    if (saveManager && cartridge) {
        saveManager->save(*cartridge);
    }
    saveManager.reset();

    cartridge->load(filePath);
    // A CGB-capable cartridge (dual-mode 0x80 or CGB-only 0xC0) enters CGB
    // mode on CGB hardware.  DMG-only carts use the DMG-compatible PPU/CPU
    // state and the CGB boot compatibility palette.  DMG/SGB models always
    // use the DMG-compatible path.
    const GbCartridgeHeader header = cartridge->cartridgeHeader();
    if (sgb) {
        sgb->setHeader(header);
    }
    const bool hardwareCgb = model == GbModel::cgb;
    const bool cgb = hardwareCgb && (header.cgb || bootRomEnabled);
    cgbMode = hardwareCgb && header.cgb;
    // The bundled image is a CGB boot ROM.  It runs on CGB hardware even for
    // DMG-only cartridges: the ROM switches to DMG compatibility by writing
    // KEY0=$04, and the CPU/PPU follow that write.
    bootRomActive = bootRomEnabled && hardwareCgb;
    bus->setCgb(cgb);
    bus->reset();
    ppu->setCgb(cgb);
    mmu->setBootRom(cgbBootRom());
    mmu->setBootRomActive(bootRomActive);
    mmu->reset();
    if (bootRomActive) {
        ppu->resetForBootRom();
    }
    ppu->setDmgMode(false);
    if (!bootRomActive && model == GbModel::cgb && !header.cgb) {
        ppu->setDmgCompatibilityPalette(qnemu::cgbCompatibilityPalette({
            header.titleBytes,
            header.oldLicensee,
            header.newLicensee,
        }));
    } else {
        ppu->clearDmgCompatibilityPalette();
    }
    cpu->reset(cgb, bootRomActive);

    saveManager = std::make_unique<SaveManager>(
        saveDirectory.empty() ? SaveManager::defaultDirectory() : saveDirectory, filePath);
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

const FrameBuffer& Gb::frameBuffer() const
{
    return ppu->frameBuffer();
}

void Gb::run()
{
    using Clock = std::chrono::steady_clock;
    constexpr auto frameInterval = std::chrono::microseconds(16743);

    bool displayEnabled = true;
    uint64_t lastFrameCount = ppu ? ppu->frameCount() : 0;
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

        cpu->step();

        const bool stopped = cpu->isStopped();
        if (display) {
            if (stopped && displayEnabled) {
                display->disable();
                displayEnabled = false;
            } else if (!stopped && !displayEnabled) {
                display->enable();
                displayEnabled = true;
            }
        }

        const uint64_t frameCount = ppu ? ppu->frameCount() : 0;
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
