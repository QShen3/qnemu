/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include "qnemu/gb/GbPlatform.h"
#include "qnemu/gb/infrared/GbInfraredLink.h"
#include "qnemu/gb/platform/GameBoyPlatform.h"
#include "qnemu/gb/serial/GbSerialLink.h"

namespace qnemu
{

class GbCartridgeInterface;
class DisplayInterface;
class InputInterface;
class SoundInterface;

std::unique_ptr<GameBoyPlatform> createPlatform(GbPlatform platform,
    std::shared_ptr<GbCartridgeInterface> cartridge,
    std::shared_ptr<DisplayInterface> display,
    std::unique_ptr<SoundInterface> sound,
    std::shared_ptr<InputInterface> input,
    std::shared_ptr<GbSerialLink> serialLink,
    std::shared_ptr<GbInfraredLink> infraredLink);

}  // namespace qnemu
