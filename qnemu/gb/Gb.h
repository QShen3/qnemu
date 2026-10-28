/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

enum class GbModel
{
    cgb,
    dmg,
    sgb,
};

class GbCpu;
class GbCpuBus;
class GbMmu;
class GbPpu;
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
    Gb(GbModel model,
        std::shared_ptr<DisplayInterface> display,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input = nullptr);
    ~Gb();

    Gb(const Gb&) = delete;
    Gb& operator=(const Gb&) = delete;

    void loadCartridge(const char* filePath);
    void stop();

private:
    void start();
    void run();

    GbModel model;
    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::unique_ptr<GbMmu> mmu;
    std::unique_ptr<GbCpuBus> bus;
    std::unique_ptr<GbCpu> cpu;
    GbPpu* ppu = nullptr;
    const GbMbcFactory mbcFactory;
    std::shared_ptr<DisplayInterface> display;
    std::shared_ptr<InputInterface> input;
    std::unique_ptr<SaveManager> saveManager;
    std::thread work;
    std::atomic_bool running{false};
};

}  // namespace qnemu
