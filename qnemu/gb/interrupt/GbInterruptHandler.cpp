/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <memory>

#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemu
{

GbInterruptHandler::GbInterruptHandler(std::shared_ptr<GbCpuInterface> cpu) : cpu(cpu)
{
    GbInterruptHandler::reset();
}

uint8_t GbInterruptHandler::read(uint16_t address) const
{
    if (address == 0xFFFF) {
        return registers.interruptEnabled;
    }
    if (address == 0xFF0F) {
        return registers.interruptFlag;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbInterruptHandler::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFFFF) {
        registers.interruptEnabled = value;
        return;
    }
    if (address == 0xFF0F) {
        registers.interruptFlag = value;
        return;
    }
    assert(false && "Wrong address");
}

void GbInterruptHandler::reset()
{
    registers.interruptFlag = 0xE1;
    registers.interruptEnabled = 0;
}

void GbInterruptHandler::step()
{
    if (!GbDeviceInterface::interruptMasterEnabled) {
        return;
    }
    uint8_t hasInterrupt = registers.interruptFlag & registers.interruptEnabled & 0x1F;
    if (hasInterrupt == 0) {
        return;
    }
    if (registers.vBlankEnabled & registers.vBlankRequest) {
        registers.vBlankRequest = 0;
        cpu.lock()->jumpToAddress(0x40);
    } else if (registers.lcdEnabled & registers.lcdRequest) {
        registers.lcdRequest = 0;
        cpu.lock()->jumpToAddress(0x48);
    } else if (registers.timerEnabled & registers.timerRequest) {
        registers.timerRequest = 0;
        cpu.lock()->jumpToAddress(0x50);
    } else if (registers.serialEnabled & registers.serialRequest) {
        registers.serialRequest = 0;
        cpu.lock()->jumpToAddress(0x58);
    } else if (registers.joyPadEnabled & registers.joyPadRequest) {
        registers.joyPadRequest = 0;
        cpu.lock()->jumpToAddress(0x60);
    }
    GbDeviceInterface::interruptMasterEnabled = false;
}

bool GbInterruptHandler::accepts(uint16_t address) const
{
    return address == 0xFF0F || address == 0xFFFF;
}

}  // namespace qnemu