/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/platform/SgbPlatform.h"

#include <memory>
#include <utility>

#include "qnemu/gb/boot/GbSgbBootRom.h"
#include "qnemu/gb/gpu/GbPpu.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

SgbPlatform::SgbPlatform(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<DisplayInterface> displayInterface,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input,
        std::shared_ptr<GbSerialLink> serialLink,
        std::shared_ptr<GbInfraredLink> infraredLink) :
    DmgPlatform(std::move(cartridge), std::move(displayInterface), std::move(sound),
        std::move(input), std::move(serialLink), std::move(infraredLink)),
    colorizer_(sgb_),
    frameSink_(sgb_, display())
{
    sgb_.reset();
    ppu()->setColorizer(&colorizer_);
    ppu()->setFrameSink(&frameSink_);
    joypad()->setWriteCallback([this](uint8_t value) {
        sgb_.write(value);
    });
}

SgbPlatform::~SgbPlatform()
{
    // The derived members (including sgb_) are destroyed before the base
    // platform and its PPU, so clear the raw hooks first.
    ppu()->setColorizer(nullptr);
    ppu()->setFrameSink(nullptr);
    joypad()->setWriteCallback({});
}

void SgbPlatform::reset(const GbCartridgeHeader& header)
{
    sgb_.setHeader(header);
    resetWithBootRom(sgbBootRom());
}

}  // namespace qnemu
