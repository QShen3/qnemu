/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <utility>

#include "qnemu/gb/platform/GameBoyPlatform.h"

namespace qnemu
{

class GbCartridgeInterface;
class GbCpu;
class GbInterruptHandler;
class GbJoypad;
class GbPpu;
class DmgCpuBus;
class DmgMmu;
class DisplayInterface;
class InputInterface;
class SoundInterface;

/**
 * Original Game Boy (DMG) platform.
 *
 * This is the shared base for SGB as well: both use the DMG CPU, PPU,
 * APU, timer and memory map.  SGB adds its command receiver and video
 * compositor on top.
 */
class DmgPlatform : public GameBoyPlatform
{
public:
    DmgPlatform(std::shared_ptr<GbCartridgeInterface> cartridge,
        std::shared_ptr<DisplayInterface> display,
        std::unique_ptr<SoundInterface> sound,
        std::shared_ptr<InputInterface> input);
    ~DmgPlatform() override;

    void reset(const GbCartridgeHeader& header) override;
    void step() override;

    const FrameBuffer& frameBuffer() const override;
    uint64_t frameCount() const override;
    bool isCpuStopped() const override;
    bool isCgbMode() const override { return cgbMode_; }
    bool isSgb() const override { return false; }
    std::pair<int, int> frameSize() const override { return {160, 144}; }

protected:
    /// Runs the DMG reset sequence with the supplied boot ROM image.
    void resetWithBootRom(std::span<const uint8_t> bootRom);

    GbPpu* ppu() const { return ppu_; }
    GbJoypad* joypad() const { return joypad_; }
    DisplayInterface& display() const { return *display_; }

    bool cgbMode_ = false;

private:
    std::shared_ptr<GbCartridgeInterface> cartridge_;
    std::shared_ptr<DisplayInterface> display_;
    std::shared_ptr<InputInterface> input_;
    std::shared_ptr<GbInterruptHandler> interruptHandler_;
    std::unique_ptr<DmgMmu> mmu_;
    std::unique_ptr<DmgCpuBus> bus_;
    std::unique_ptr<GbCpu> cpu_;
    GbPpu* ppu_ = nullptr;
    GbJoypad* joypad_ = nullptr;
};

}  // namespace qnemu
