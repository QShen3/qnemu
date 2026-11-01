/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/platform/DmgPlatform.h"

#include <array>
#include <memory>
#include <utility>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "qnemu/gb/boot/GbDmgBootRom.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/DmgPpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/DmgWorkRam.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/mmu/DmgCpuBus.h"
#include "qnemu/gb/mmu/DmgMmu.h"
#include "qnemu/gb/serial/GbSerial.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

DmgPlatform::DmgPlatform(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input,
        std::shared_ptr<GbSerialLink> serialLink,
        std::shared_ptr<GbInfraredLink> /*infraredLink*/) :
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
        false, GbApu::DefaultFadeInFrames);

    interruptHandler_ = std::make_shared<GbInterruptHandler>();
    auto workRam = std::make_unique<DmgWorkRam>();
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

    auto ppu = std::make_unique<DmgPpu>(display_, interruptHandler_);
    ppu_ = ppu.get();
    ppu_->setOamBugEnabled(true);
    auto serial = std::make_unique<GbSerial>(interruptHandler_, std::move(serialLink), false);
    mmu_ = std::make_unique<DmgMmu>(std::move(apu), cartridge_, std::move(ppu),
        std::move(highRam), interruptHandler_, std::move(joypad), std::move(workRam),
        std::move(timer), std::move(serial));
    ppu_->setExternalRead([mmuPointer = mmu_.get()](uint16_t address) {
        return mmuPointer->read(address);
    });
    bus_ = std::make_unique<DmgCpuBus>(*mmu_);
    cpu_ = std::make_unique<GbCpu>(*bus_);
}

DmgPlatform::~DmgPlatform()
{
    if (input_) {
        input_->setButtonPressCallback({});
        input_->setButtonReleaseCallback({});
    }
}

void DmgPlatform::reset(const GbCartridgeHeader& /*header*/)
{
    cgbMode_ = false;
    resetWithBootRom(dmgBootRom());
}

void DmgPlatform::resetWithBootRom(std::span<const uint8_t> bootRom)
{
    bus_->reset();
    mmu_->setBootRom(bootRom);
    mmu_->setBootRomActive(true);
    mmu_->reset();
    ppu_->resetForBootRom();
    cpu_->reset(false, true);
}

void DmgPlatform::step()
{
    cpu_->step();
}

const FrameBuffer& DmgPlatform::frameBuffer() const
{
    return ppu_->frameBuffer();
}

uint64_t DmgPlatform::frameCount() const
{
    return ppu_->frameCount();
}

bool DmgPlatform::isCpuStopped() const
{
    return cpu_->isStopped();
}

}  // namespace qnemu
