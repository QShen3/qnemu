/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <memory>
#include <utility>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbCpuBus.h"
#include "qnemu/gb/mmu/GbMmu.h"
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
    auto apu = std::make_unique<GbApu>(std::move(soundInterface), std::move(channels),
        model == GbModel::cgb, GbApu::DefaultFadeInFrames);

    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    auto interruptHandler = std::make_shared<GbInterruptHandler>();

    auto workRam = std::make_unique<GbWorkRam>();
    auto highRam = std::make_unique<GbHighRam>();
    auto timer = std::make_unique<GbTimer>(interruptHandler);

    auto joypad = std::make_unique<GbJoypad>(interruptHandler);
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

    mmu = std::make_unique<GbMmu>(std::move(apu), cartridge, std::move(ppuInstance), std::move(highRam),
        interruptHandler, std::move(joypad), std::move(workRam), std::move(timer));
    ppu->setExternalRead([mmuPointer = mmu.get()](uint16_t address) {
        return mmuPointer->read(address);
    });
    bus = std::make_unique<GbCpuBus>(
        *mmu, model == GbModel::cgb && cartridge->isGbcCartridge());
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
    // CGB hardware only enters CGB mode for a CGB cartridge. DMG/SGB models
    // always use the DMG-compatible PPU/CPU state.
    const bool cgb = model == GbModel::cgb && cartridge->isGbcCartridge();
    bus->setCgb(cgb);
    bus->reset();
    ppu->setCgb(cgb);
    mmu->reset();
    cpu->reset(cgb);

    saveManager = std::make_unique<SaveManager>(
        SaveManager::defaultDirectory(), filePath);
    saveManager->load(*cartridge);

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
    if (work.joinable()) {
        work.join();
    }
}

void Gb::run()
{
    bool displayEnabled = true;
    while (running.load()) {
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
    }
}

}  // namespace qnemu
