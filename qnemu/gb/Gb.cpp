/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/gpu/GbGpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"

namespace qnemu
{

Gb::Gb()
{
    cpu = std::make_shared<GbCpu>();
    auto gbInterruptHandler = std::make_shared<GbInterruptHandler>(cpu);

    cartridge = std::make_shared<GbCartridge>(mbcFactory);

    gpu = std::make_shared<GbGpu>(*cartridge, gbInterruptHandler);

    auto gbWorkRam = std::make_shared<GbWorkRam>();
    auto gbHighRam = std::make_shared<GbHighRam>();

    cpu->addDevice(cartridge);
    cpu->addDevice(gpu);
    cpu->addDevice(gbWorkRam);
    cpu->addDevice(gbHighRam);
    cpu->addDevice(gbInterruptHandler);
}

void Gb::loadCartridge(const char* filePath)
{
    if (cpu) {
        cpu->stop();
        cpu->reset();
        cartridge->load(filePath);
        cpu->start();
    }
}

void Gb::stop()
{
    if (cpu) {
        cpu->stop();
    }
}

void Gb::setDisplay(std::shared_ptr<DisplayInterface> display)
{
    if (gpu) {
        gpu->setDisplay(display);
    }
}

}  // namespace qnemu