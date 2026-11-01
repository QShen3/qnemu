/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <utility>

#include "qnemu/display/FrameBuffer.h"
#include "qnemu/gb/cartridge/GbCartridgeHeader.h"

namespace qnemu
{

/**
 * One assembled Game Boy hardware platform.
 *
 * `Gb` owns the emulation thread, save service and host interfaces.  A
 * platform owns the device graph and the platform-specific boot/reset
 * sequence, so the host never has to branch on DMG/SGB/CGB.
 */
class GameBoyPlatform
{
public:
    virtual ~GameBoyPlatform() = default;

    GameBoyPlatform(const GameBoyPlatform&) = delete;
    GameBoyPlatform& operator=(const GameBoyPlatform&) = delete;

    /// Prepares the platform for the supplied cartridge header.  This runs the
    /// platform's boot ROM and resets every device.
    virtual void reset(const GbCartridgeHeader& header) = 0;

    /// Executes one CPU instruction and advances all peripherals.
    virtual void step() = 0;

    virtual const FrameBuffer& frameBuffer() const = 0;
    virtual uint64_t frameCount() const = 0;
    virtual bool isCpuStopped() const = 0;
    virtual bool isCgbMode() const = 0;
    virtual bool isSgb() const = 0;
    virtual std::pair<int, int> frameSize() const = 0;

protected:
    GameBoyPlatform() = default;
};

}  // namespace qnemu
