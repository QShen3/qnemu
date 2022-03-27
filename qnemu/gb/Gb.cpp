/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
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

    GbMbcFactory mbcFactory;
    cartridge = std::make_shared<GbCartridge>(mbcFactory);

    auto gbGpu = std::make_shared<GbGpu>(gbInterruptHandler);

    auto gbWorkRam = std::make_shared<GbWorkRam>();
    auto gbHighRam = std::make_shared<GbHighRam>();

    cpu->addDevice(cartridge);
    cpu->addDevice(gbGpu);
    cpu->addDevice(gbWorkRam);
    cpu->addDevice(gbHighRam);
    cpu->addDevice(gbInterruptHandler);
}

void Gb::loadCartridge(const char* filePath)
{
    cpu->stop();
    cpu->reset();
    cartridge->load(filePath);
    cpu->start();
}

}  // namespace qnemu