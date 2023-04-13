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
    auto interruptHandler = std::make_shared<GbInterruptHandler>();

    cpu = std::make_shared<GbCpu>(interruptHandler);

    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    auto rasterDisplay = std::make_shared<RasterDisplay>();

    auto gbPalette = std::make_unique<GbPalette>();
    auto gbcPalette = std::make_unique<GbcPalette>();
    auto gbOam = std::make_unique<GbOam>(cpu);
    auto gbVideoRam = std::make_unique<GbVideoRam>(*cartridge, cpu);
    auto gpu = std::make_shared<GbGpu>(*cartridge, rasterDisplay, interruptHandler, std::move(gbPalette), std::move(gbcPalette), std::move(gbOam), std::move(gbVideoRam));

    auto workRam = std::make_shared<GbWorkRam>();
    auto highRam = std::make_shared<GbHighRam>();
    auto timer = std::make_shared<GbTimer>(interruptHandler);
    auto joypad = std::make_shared<GbJoypad>(rasterDisplay, interruptHandler);

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