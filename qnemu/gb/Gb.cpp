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
#include "qnemu/gb/gpu/GbOam.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbMmu.h"
#include "qnemu/gb/timer/GbTimer.h"

namespace qnemu
{

Gb::Gb()
{
    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    auto rasterDisplay = std::make_shared<RasterDisplay>();
    auto workRam = std::make_shared<GbWorkRam>();
    auto highRam = std::make_shared<GbHighRam>();
    auto interruptHandler = std::make_shared<GbInterruptHandler>();
    auto timer = std::make_shared<GbTimer>(interruptHandler);
    auto joypad = std::make_shared<GbJoypad>(rasterDisplay, interruptHandler);

    cpu = std::make_shared<GbCpu>(interruptHandler);

    auto palette = std::make_unique<GbPalette>();
    auto coloredPalette = std::make_unique<GbcPalette>();
    auto gbVideoRam = std::make_shared<GbVideoRam>(*cartridge, *workRam);
    auto oam = std::make_unique<GbOam>(*cartridge, *gbVideoRam, *workRam);
    auto gpu = std::make_shared<GbGpu>(*cartridge, rasterDisplay, interruptHandler, std::move(palette), std::move(coloredPalette), std::move(oam), gbVideoRam);

    auto mmu = std::make_unique<GbMmu>(cartridge, gpu, highRam, interruptHandler, joypad, workRam, timer);

    cpu->addDisplay(rasterDisplay);
    cpu->addMmu(std::move(mmu));

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