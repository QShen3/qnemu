/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/memory/GbHighRam.h"

namespace qnemu
{

Gb::Gb()
{
    cpu = std::make_shared<GbCpu>();

    GbMbcFactory mbcFactory;
    cartridge = std::make_shared<GbCartridge>(mbcFactory);

    auto gbInterruptHandler = std::make_shared<GbInterruptHandler>(cpu);

    auto gbHighRam = std::make_shared<GbHighRam>();

    cpu->addDevice(cartridge);
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