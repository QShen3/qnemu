/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include "qnemu/gb/gpu/GbPpu.h"

namespace qnemu
{

class CgbPpuVideo;
class DisplayInterface;
class GbInterruptHandlerInterface;

/// CGB PPU: timing core plus the CGB video path and DMG compatibility mode.
class CgbPpu final : public GbPpu
{
public:
    CgbPpu(std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~CgbPpu() override = default;

    void setDmgMode(bool dmgMode) override;
    void enableDmgCompatibility() override;

private:
    CgbPpuVideo* video_ = nullptr;
};

}  // namespace qnemu
