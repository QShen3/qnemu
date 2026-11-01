/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/gpu/DmgPpu.h"

#include <memory>
#include <utility>

#include "qnemu/gb/gpu/DmgPpuVideo.h"

namespace qnemu
{

DmgPpu::DmgPpu(std::shared_ptr<DisplayInterface> display,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler) :
    GbPpu(std::make_unique<DmgPpuVideo>(), std::move(display), std::move(interruptHandler))
{
}

}  // namespace qnemu
