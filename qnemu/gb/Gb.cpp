/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <utility>

#include "qnemu/display/RasterDisplay.h"
#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

Gb::Gb()
{
    cpu = std::make_shared<GbCpu>();

    auto interruptHandler = std::make_shared<GbInterruptHandler>(cpu);
    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    auto rasterDisplay = std::make_shared<RasterDisplay>();
    auto gpu = std::make_shared<GbGpu>(*cartridge, rasterDisplay, interruptHandler);

    auto gbPalette = std::make_unique<GbPalette>();
    auto gbcPalette = std::make_unique<GbcPalette>();
    auto spriteAttributeTable = std::make_unique<SpriteAttributeTable>(cpu, *gpu);
    auto gbVideoRam = std::make_unique<GbVideoRam>(*cartridge, cpu, *gpu);
    
    gpu->addGbPalette(std::move(gbPalette));
    gpu->addGbcPalette(std::move(gbcPalette));
    gpu->addSpriteAttributeTable(std::move(spriteAttributeTable));
    gpu->addGbVideoRam(std::move(gbVideoRam));

    auto workRam = std::make_shared<GbWorkRam>();
    auto highRam = std::make_shared<GbHighRam>();

    auto timer = std::make_shared<GbTimer>(interruptHandler);
    auto joypad = std::make_shared<GbJoypad>(rasterDisplay, interruptHandler);

    cpu->addDevice(cartridge);
    cpu->addDevice(gpu);
    cpu->addDevice(workRam);
    cpu->addDevice(highRam);
    cpu->addDevice(timer);
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