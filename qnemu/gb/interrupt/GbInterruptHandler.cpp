/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>

#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemu
{

GbInterruptHandler::GbInterruptHandler()
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

void GbInterruptHandler::step()
{
    if (!GbDeviceInterface::interruptMasterEnabled) {
        return;
    }
    const uint8_t hasInterrupt = registers.interruptFlag & registers.interruptEnabled & 0x1F;
    if (hasInterrupt == 0) {
        return;
    }
    if (registers.vBlankEnabled & registers.vBlankRequest) {
        if (cpuCallback) {
            cpuCallback(GbInterrupt::vBlank);
        }
        if (!registers.vBlankEnabled) {
            if (cpuCallback) {
                cpuCallback(GbInterrupt::cancel);
            }
        } else {
            registers.vBlankRequest = 0;
            GbDeviceInterface::interruptMasterEnabled = false;
            return;
        }
    }
    if (registers.lcdEnabled & registers.lcdRequest) {
        if (cpuCallback) {
            cpuCallback(GbInterrupt::lcd);
        }
        if (!registers.lcdEnabled) {
            if (cpuCallback) {
                cpuCallback(GbInterrupt::cancel);
            }
        } else {
            registers.lcdRequest = 0;
            GbDeviceInterface::interruptMasterEnabled = false;
            return;
        }
    }
    if (registers.timerEnabled & registers.timerRequest) {
        if (cpuCallback) {
            cpuCallback(GbInterrupt::timer);
        }
        if (!registers.timerEnabled) {
            if (cpuCallback) {
                cpuCallback(GbInterrupt::cancel);
            }
        } else {
            registers.timerRequest = 0;
            GbDeviceInterface::interruptMasterEnabled = false;
            return;
        }
    }
    if (registers.serialEnabled & registers.serialRequest) {
        if (cpuCallback) {
            cpuCallback(GbInterrupt::serial);
        }
        registers.serialRequest = 0;
        GbDeviceInterface::interruptMasterEnabled = false;
        return;
    }
    if (registers.joyPadEnabled & registers.joyPadRequest) {
        if (cpuCallback) {
            cpuCallback(GbInterrupt::joypad);
        }
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

void GbInterruptHandler::registerCpuCallback(std::function<void(GbInterrupt)> cpuCallback)
{
    this->cpuCallback = cpuCallback;
}

void GbInterruptHandler::requestVBlankInterrupt()
{
    registers.vBlankRequest = 1;
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitHalt);
    }
}

void GbInterruptHandler::requestLcdInterrupt()
{
    registers.lcdRequest = 1;
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitHalt);
    }
}

void GbInterruptHandler::requestTimerInterrupt()
{
    registers.timerRequest = 1;
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitHalt);
    }
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitStop);
    }
}

void GbInterruptHandler::requestSerialInterrupt()
{
    registers.serialRequest = 1;
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitHalt);
    }
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitStop);
    }
}

void GbInterruptHandler::requestJoypadInterrupt()
{
    registers.joyPadRequest = 1;
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitHalt);
    }
    if (cpuCallback) {
        cpuCallback(GbInterrupt::exitStop);
    }
}

}  // namespace qnemu