/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include "qnemu/gb/platform/GameBoyPlatform.h"
#include "qnemu/gb/infrared/GbInfraredLink.h"
#include "qnemu/gb/serial/GbSerialLink.h"

namespace qnemu
{

class GbCartridgeInterface;
class GbCpu;
class GbInterruptHandler;
class GbJoypad;
class CgbCpuBus;
class CgbMmu;
class CgbPpu;
class DisplayInterface;
class InputInterface;
class SoundInterface;

/**
 * Game Boy Color (CGB) platform.
 *
 * The CGB is a superset of the DMG.  The platform always boots the CGB boot
 * ROM: dual-mode and CGB-only cartridges stay in native CGB mode, while
 * DMG-only cartridges switch to the CGB's DMG compatibility mode through
 * KEY0.
 */
class CgbPlatform final : public GameBoyPlatform
{
public:
    CgbPlatform(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input,
        std::shared_ptr<GbSerialLink> serialLink,
        std::shared_ptr<GbInfraredLink> infraredLink);
    ~CgbPlatform() override;

    void reset(const GbCartridgeHeader& header) override;
    void step() override;

    const FrameBuffer& frameBuffer() const override;
    uint64_t frameCount() const override;
    bool isCpuStopped() const override;
    bool isCgbMode() const override { return cgbMode_; }
    bool isSgb() const override { return false; }
    std::pair<int, int> frameSize() const override { return {160, 144}; }

private:
    std::shared_ptr<GbCartridgeInterface> cartridge_;
    std::shared_ptr<DisplayInterface> display_;
    std::shared_ptr<InputInterface> input_;
    std::shared_ptr<GbInterruptHandler> interruptHandler_;
    std::unique_ptr<CgbMmu> mmu_;
    std::unique_ptr<CgbCpuBus> bus_;
    std::unique_ptr<GbCpu> cpu_;
    CgbPpu* ppu_ = nullptr;
    GbJoypad* joypad_ = nullptr;
    bool cgbMode_ = false;
};

}  // namespace qnemu
