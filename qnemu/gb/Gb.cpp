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
    auto gbInterruptHandler = std::make_shared<GbInterruptHandler>(cpu);

    cartridge = std::make_shared<GbCartridge>(mbcFactory);

    auto rasterDisplay = std::make_shared<RasterDisplay>();
    auto gpu = std::make_shared<GbGpu>(*cartridge, cpu, rasterDisplay, gbInterruptHandler);

    auto gbWorkRam = std::make_shared<GbWorkRam>();
    auto gbHighRam = std::make_shared<GbHighRam>();

    auto joypad = std::make_shared<GbJoypad>(rasterDisplay);

    cpu->addDevice(cartridge);
    cpu->addDevice(gpu);
    cpu->addDevice(gbWorkRam);
    cpu->addDevice(gbHighRam);
    cpu->addDevice(joypad);
    cpu->addDevice(gbInterruptHandler);

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