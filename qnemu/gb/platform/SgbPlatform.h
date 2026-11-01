/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>
#include <utility>

#include "qnemu/gb/platform/DmgPlatform.h"
#include "qnemu/gb/infrared/GbInfraredLink.h"
#include "qnemu/gb/serial/GbSerialLink.h"
#include "qnemu/gb/sgb/GbSgb.h"
#include "qnemu/gb/sgb/SgbVideo.h"

namespace qnemu
{

class GbCartridgeInterface;
class DisplayInterface;
class InputInterface;
class SoundInterface;

/**
 * Super Game Boy platform.
 *
 * The SGB is a DMG with an SNES-side command/palette/border peripheral.  It
 * reuses the complete DMG device graph and adds the SGB receiver and video
 * path.
 */
class SgbPlatform final : public DmgPlatform
{
public:
    SgbPlatform(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<DisplayInterface> displayInterface,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input,
        std::shared_ptr<GbSerialLink> serialLink,
        std::shared_ptr<GbInfraredLink> infraredLink);
    ~SgbPlatform() override;

    void reset(const GbCartridgeHeader& header) override;

    bool isSgb() const override { return true; }
    std::pair<int, int> frameSize() const override { return {256, 224}; }

private:
    GbSgb sgb_;
    SgbColorizer colorizer_;
    SgbFrameSink frameSink_;
};

}  // namespace qnemu
