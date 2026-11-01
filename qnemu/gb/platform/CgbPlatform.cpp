/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/platform/CgbPlatform.h"

#include <array>
#include <memory>
#include <utility>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "qnemu/gb/boot/GbCgbBootRom.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/CgbWorkRam.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/mmu/CgbCpuBus.h"
#include "qnemu/gb/mmu/CgbMmu.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

CgbPlatform::CgbPlatform(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input) :
    cartridge_(std::move(cartridge)),
    display_(std::move(display)),
    input_(std::move(input))
{
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels {
        std::make_unique<GbChannel1>(),
        std::make_unique<GbChannel2>(),
        std::make_unique<GbChannel3>(),
        std::make_unique<GbChannel4>(),
    };
    auto apu = std::make_unique<GbApu>(std::move(sound), std::move(channels),
        true, GbApu::DefaultFadeInFrames);

    interruptHandler_ = std::make_shared<GbInterruptHandler>();
    auto workRam = std::make_unique<CgbWorkRam>();
    auto highRam = std::make_unique<GbHighRam>();
    auto timer = std::make_unique<GbTimer>(interruptHandler_);
    auto joypad = std::make_unique<GbJoypad>(interruptHandler_);
    joypad_ = joypad.get();
    if (input_) {
        input_->setButtonPressCallback([joypadPointer = joypad_](GbButton button) {
            joypadPointer->setButtonState(button, true);
        });
        input_->setButtonReleaseCallback([joypadPointer = joypad_](GbButton button) {
            joypadPointer->setButtonState(button, false);
        });
    }

    auto ppu = std::make_unique<GbPpu>(true, display_, interruptHandler_);
    ppu_ = ppu.get();
    ppu_->setOamBugEnabled(false);
    mmu_ = std::make_unique<CgbMmu>(std::move(apu), cartridge_, std::move(ppu),
        std::move(highRam), interruptHandler_, std::move(joypad), std::move(workRam),
        std::move(timer));
    ppu_->setExternalRead([mmuPointer = mmu_.get()](uint16_t address) {
        return mmuPointer->read(address);
    });
    bus_ = std::make_unique<CgbCpuBus>(*mmu_);
    bus_->setCgbModeCallback([this](bool mode) {
        cgbMode_ = mode;
        ppu_->setDmgMode(!mode);
        if (!mode) {
            ppu_->enableDmgCompatibility();
        }
    });
    cpu_ = std::make_unique<GbCpu>(*bus_);
}

CgbPlatform::~CgbPlatform()
{
    if (input_) {
        input_->setButtonPressCallback({});
        input_->setButtonReleaseCallback({});
    }
}

void CgbPlatform::reset(const GbCartridgeHeader& header)
{
    cgbMode_ = header.cgb;
    bus_->setCgb(true);
    bus_->reset();
    ppu_->setCgb(true);
    mmu_->setBootRom(cgbBootRom());
    mmu_->setBootRomActive(true);
    mmu_->reset();
    ppu_->resetForBootRom();
    ppu_->setDmgMode(false);
    cpu_->reset(true, true);
}

void CgbPlatform::step()
{
    cpu_->step();
}

const FrameBuffer& CgbPlatform::frameBuffer() const
{
    return ppu_->frameBuffer();
}

uint64_t CgbPlatform::frameCount() const
{
    return ppu_->frameCount();
}

bool CgbPlatform::isCpuStopped() const
{
    return cpu_->isStopped();
}

}  // namespace qnemu
