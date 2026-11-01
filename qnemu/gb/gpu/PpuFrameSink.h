/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <span>

namespace qnemu
{

class FrameBuffer;

/**
 * Optional end-of-frame presentation hook.
 *
 * The default path presents the 160x144 DMG/CGB frame directly.  SGB installs
 * a sink that composes its 256x224 border and forwards that to the display,
 * keeping the SGB frame format out of the PPU.
 */
class PpuFrameSink
{
public:
    virtual ~PpuFrameSink() = default;

    virtual void present(const FrameBuffer& frame,
        std::span<const uint8_t> vram) = 0;
};

}  // namespace qnemu
