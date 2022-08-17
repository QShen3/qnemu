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

bool GbInterruptHandler::accepts(uint16_t address) const
{
    return address == 0xFF0F || address == 0xFFFF;
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
        cpu.lock()->jumpToAddress(0x40);
        if (!registers.vBlankEnabled) {
            cpu.lock()->cancelInterrupt();
        } else {
            registers.vBlankRequest = 0;
            GbDeviceInterface::interruptMasterEnabled = false;
            return;
        }
    }
    if (registers.lcdEnabled & registers.lcdRequest) {
        cpu.lock()->jumpToAddress(0x48);
        if (!registers.lcdEnabled) {
            cpu.lock()->cancelInterrupt();
        } else {
            registers.lcdRequest = 0;
            GbDeviceInterface::interruptMasterEnabled = false;
            return;
        }
    }
    if (registers.timerEnabled & registers.timerRequest) {
        cpu.lock()->jumpToAddress(0x50);
        if (!registers.timerEnabled) {
            cpu.lock()->cancelInterrupt();
        } else {
            registers.timerRequest = 0;
            GbDeviceInterface::interruptMasterEnabled = false;
            return;
        }
    }
    if (registers.serialEnabled & registers.serialRequest) {
        cpu.lock()->jumpToAddress(0x58);
        registers.serialRequest = 0;
        GbDeviceInterface::interruptMasterEnabled = false;
        return;
    }
    if (registers.joyPadEnabled & registers.joyPadRequest) {
        cpu.lock()->jumpToAddress(0x60);
        registers.joyPadRequest = 0;
        GbDeviceInterface::interruptMasterEnabled = false;
        return;
    }
    GbDeviceInterface::interruptMasterEnabled = false;
}

void GbInterruptHandler::reset()
{
    registers.interruptFlag = 0xE1;
    registers.interruptEnabled = 0;
}

void GbInterruptHandler::requestVBlankInterrupt()
{
    registers.vBlankRequest = 1;
}

void GbInterruptHandler::requestLcdInterrupt()
{
    registers.lcdRequest = 1;
}

void GbInterruptHandler::requestTimerInterrupt()
{
    registers.timerRequest = 1;
}

void GbInterruptHandler::requestSerialInterrupt()
{
    registers.serialRequest = 1;
}

void GbInterruptHandler::requestJoypadInterrupt()
{
    registers.joyPadRequest = 1;
}

}  // namespace qnemu