/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <utility>

#include "qnemu/display/RasterDisplay.h"
#include "qnemu/gb/Gb.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/gpu/GbPalette.h"
#include "qnemu/gb/gpu/GbVideoRam.h"
#include "qnemu/gb/gpu/GbcPalette.h"
#include "qnemu/gb/gpu/GbOam.h"
#include "qnemu/gb/gpu/GbRender.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbMmu.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

Gb::Gb()
{
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels {
        std::make_unique<GbChannel1>(),
        std::make_unique<GbChannel2>(),
        std::make_unique<GbChannel3>(),
        std::make_unique<GbChannel4>(),
    };
    auto sound = std::make_unique<DefaultSound>(44100, "2.0", 8);
    auto apu = std::make_unique<GbApu>(std::move(sound), std::move(channels));

    cartridge = std::make_shared<GbCartridge>(mbcFactory);
    auto rasterDisplay = std::make_shared<RasterDisplay>();
    auto interruptHandler = std::make_shared<GbInterruptHandler>();

    auto workRam = std::make_unique<GbWorkRam>();
    auto highRam = std::make_unique<GbHighRam>();
    auto timer = std::make_unique<GbTimer>(interruptHandler);
    auto joypad = std::make_unique<GbJoypad>(rasterDisplay, interruptHandler);

    auto palette = std::make_unique<GbPalette>();
    auto coloredPalette = std::make_unique<GbcPalette>();
    auto videoRam = std::make_unique<GbVideoRam>(*cartridge, *workRam);
    auto oam = std::make_unique<GbOam>(*cartridge, *videoRam, *workRam);
    auto render = std::make_unique<GbRender>(*cartridge, rasterDisplay, interruptHandler, *palette, *coloredPalette, *oam, *videoRam);
    auto gpu = std::make_unique<GbGpu>(std::move(palette), std::move(coloredPalette), std::move(oam), std::move(videoRam), std::move(render));

    auto mmu = std::make_unique<GbMmu>(std::move(apu), cartridge, std::move(gpu), std::move(highRam), interruptHandler, std::move(joypad), std::move(workRam), std::move(timer));

    cpu = std::make_shared<GbCpu>(interruptHandler, std::move(mmu));
    cpu->setDisplay(rasterDisplay);

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