/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>

#include <QtCore/qnamespace.h>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"

namespace qnemu
{

GbJoypad::GbJoypad(std::shared_ptr<DisplayInterface> display, std::shared_ptr<GbInterruptHandler> interruptHandler)
    : display(display), interruptHandler(interruptHandler)
{
    GbJoypad::reset();
    display->setKeyPressCallback(std::bind(&GbJoypad::processKeyPressEvent, this, std::placeholders::_1));
    display->setKeyReleaseCallback(std::bind(&GbJoypad::processKeyReleaseEvent, this, std::placeholders::_1));
}

bool GbJoypad::accepts(uint16_t address) const
{
    return address == 0xFF00;
}

uint8_t GbJoypad::read(uint16_t address) const
{
    if (address == 0xFF00) {
        std::lock_guard lock(mutex);
        return registers.joypadState;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbJoypad::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF00) {
        std::lock_guard lock(mutex);
        registers.joypadState = ((value & 0b11110000) | (registers.joypadState & 0b00001111));
        return;
    }
    assert(false && "Wrong address");
}

void GbJoypad::step()
{
}

void GbJoypad::reset()
{
    registers.joypadState = 0xCF;
}

void GbJoypad::processKeyPressEvent(int key)
{
    std::lock_guard lock(mutex);
    if ((registers.joypadState & 0x10) == 0) {
        if (key == Qt::Key_S) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b1000;
        } else if (key == Qt::Key_W) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b100;
        } else if (key == Qt::Key_A) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b10;
        } else if (key == Qt::Key_D) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b1;
        }
    } else if ((registers.joypadState & 0x20) == 0) {
        if (key == Qt::Key_3) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b1000;
        } else if (key == Qt::Key_2) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b100;
        } else if (key == Qt::Key_K) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b10;
        } else if (key == Qt::Key_J) {
            interruptHandler->registers.joyPadRequest = 1;
            registers.joypadState &= ~0b1;
        }
    }
}

void GbJoypad::processKeyReleaseEvent(int key)
{
    std::lock_guard lock(mutex);
    if ((registers.joypadState & 0x10) == 0) {
        if (key == Qt::Key_S) {
            registers.joypadState |= 0b1000;
        } else if (key == Qt::Key_W) {
            registers.joypadState |= 0b100;
        } else if (key == Qt::Key_A) {
            registers.joypadState |= 0b10;
        } else if (key == Qt::Key_D) {
            registers.joypadState |= 0b1;
        }
    } else if ((registers.joypadState & 0x20) == 0) {
        if (key == Qt::Key_3) {
            registers.joypadState |= 0b1000;
        } else if (key == Qt::Key_2) {
            registers.joypadState |= 0b100;
        } else if (key == Qt::Key_K) {
            registers.joypadState |= 0b10;
        } else if (key == Qt::Key_J) {
            registers.joypadState |= 0b1;
        }
    }
}

}  // namespace qnemu
