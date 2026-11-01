/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/platform/PlatformFactory.h"

#include <utility>

#include "qnemu/gb/platform/CgbPlatform.h"
#include "qnemu/gb/platform/DmgPlatform.h"
#include "qnemu/gb/platform/SgbPlatform.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

std::unique_ptr<GameBoyPlatform> createPlatform(GbPlatform platform,
    std::shared_ptr<GbCartridgeInterface> cartridge,
    std::shared_ptr<DisplayInterface> display,
    std::unique_ptr<SoundInterface> sound,
    std::shared_ptr<InputInterface> input,
    std::shared_ptr<GbSerialLink> serialLink,
    std::shared_ptr<GbInfraredLink> infraredLink)
{
    switch (platform) {
        case GbPlatform::gb:
            return std::make_unique<DmgPlatform>(std::move(cartridge), std::move(display),
                std::move(sound), std::move(input), std::move(serialLink),
                std::move(infraredLink));
        case GbPlatform::sgb:
            return std::make_unique<SgbPlatform>(std::move(cartridge), std::move(display),
                std::move(sound), std::move(input), std::move(serialLink),
                std::move(infraredLink));
        case GbPlatform::gbc:
            return std::make_unique<CgbPlatform>(std::move(cartridge), std::move(display),
                std::move(sound), std::move(input), std::move(serialLink),
                std::move(infraredLink));
    }
    return nullptr;
}

}  // namespace qnemu
