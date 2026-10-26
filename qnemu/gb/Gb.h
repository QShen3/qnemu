/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <QtGui/QWindow>

#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"

namespace qnemu
{

class GbCpu;
class GbCpuBus;
class GbMmu;
class RasterDisplay;

class Gb
{
public:
    Gb();
    ~Gb();

    Gb(const Gb&) = delete;
    Gb& operator=(const Gb&) = delete;

    void loadCartridge(const char* filePath);
    std::shared_ptr<QWindow> getDisplay();
    void stop();

private:
    void start();
    void run();

    std::shared_ptr<GbCartridgeInterface> cartridge;
    std::unique_ptr<GbMmu> mmu;
    std::unique_ptr<GbCpuBus> bus;
    std::unique_ptr<GbCpu> cpu;
    const GbMbcFactory mbcFactory;
    std::shared_ptr<RasterDisplay> display;
    std::thread work;
    std::atomic_bool running{false};
};

}  // namespace qnemu
