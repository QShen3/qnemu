/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include "qnemu/gb/gpu/GbPpu.h"

namespace qnemu
{

class DisplayInterface;
class GbInterruptHandlerInterface;

/// DMG PPU: timing core plus the DMG video path.
class DmgPpu final : public GbPpu
{
public:
    DmgPpu(std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~DmgPpu() override = default;
};

}  // namespace qnemu
