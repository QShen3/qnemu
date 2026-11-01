/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/gpu/CgbPpu.h"

#include <memory>
#include <utility>

#include "qnemu/gb/gpu/CgbPpuVideo.h"

namespace qnemu
{

CgbPpu::CgbPpu(std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) :
    GbPpu(std::make_unique<CgbPpuVideo>(), std::move(display), std::move(interruptHandler))
{
    video_ = static_cast<CgbPpuVideo*>(&video());
}

void CgbPpu::setDmgMode(bool dmgMode)
{
    video_->setDmgMode(dmgMode);
}

void CgbPpu::enableDmgCompatibility()
{
    memory().enableDmgCompatibility();
}

}  // namespace qnemu
