/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include "qnemu/display/RasterDisplay.h"
#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemu
{

Gb::Gb()
{
    cpu = std::make_shared<GbCpu>();
    auto interruptHandler = std::make_shared<GbInterruptHandler>(cpu);

    cartridge = std::make_shared<GbCartridge>(mbcFactory);

    auto rasterDisplay = std::make_shared<RasterDisplay>();
    auto gpu = std::make_shared<GbGpu>(*cartridge, cpu, rasterDisplay, interruptHandler);

    auto workRam = std::make_shared<GbWorkRam>();
    auto highRam = std::make_shared<GbHighRam>();

    auto joypad = std::make_shared<GbJoypad>(rasterDisplay, interruptHandler);

    cpu->addDevice(cartridge);
    cpu->addDevice(gpu);
    cpu->addDevice(workRam);
    cpu->addDevice(highRam);
    cpu->addDevice(joypad);
    cpu->addDevice(interruptHandler);

    display = rasterDisplay;
}

void Gb::loadCartridge(const char* filePath)
{
    if (cpu) {
        cpu->stop();
        cpu->reset();
        cartridge->load(filePath);
        cpu->start();
    }
}

void Gb::stop()
{
    if (cpu) {
        cpu->stop();
    }
}

std::shared_ptr<QWindow> Gb::getDisplay()
{
    return display;
}

}  // namespace qnemu