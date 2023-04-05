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
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/joypad/GbJoypad.h"

namespace qnemu
{

GbJoypad::GbJoypad(std::shared_ptr<DisplayInterface> display, std::shared_ptr<GbInterruptHandlerInterface> interruptHandler)
    : display(display), interruptHandler(interruptHandler)
{
    GbJoypad::reset();
    display->setKeyPressCallback(std::bind(&GbJoypad::processKeyPressEvent, this, std::placeholders::_1));
    display->setKeyReleaseCallback(std::bind(&GbJoypad::processKeyReleaseEvent, this, std::placeholders::_1));
}

uint8_t GbJoypad::read(uint16_t address) const
{
    if (address == 0xFF00) {
        std::lock_guard lock(mutex);
        if (registers.selectDirectionButtons == 0) {
            registers.joypadState = ((registers.joypadState & 0xF0) | directionButtionState.data);
        } else if (registers.selectActionButtons == 0) {
            registers.joypadState = ((registers.joypadState & 0xF0) | actionButtonsState.data);
        }
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

    actionButtonsState.data = 0xF;
    directionButtionState.data = 0xF;
}

void GbJoypad::processKeyPressEvent(int key)
{
    std::lock_guard lock(mutex);
    if (key == Qt::Key_S) {
        directionButtionState.down = 0;
        if (registers.selectDirectionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else if (key == Qt::Key_W) {
        directionButtionState.up = 0;
        if (registers.selectDirectionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else if (key == Qt::Key_A) {
        directionButtionState.left = 0;
        if (registers.selectDirectionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else if (key == Qt::Key_D) {
        directionButtionState.right = 0;
        if (registers.selectDirectionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else  if (key == Qt::Key_3) {
        actionButtonsState.start = 0;
        if (registers.selectActionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else if (key == Qt::Key_2) {
        actionButtonsState.select = 0;
        if (registers.selectActionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else if (key == Qt::Key_K) {
        actionButtonsState.b = 0;
        if (registers.selectActionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    } else if (key == Qt::Key_J) {
        actionButtonsState.a = 0;
        if (registers.selectActionButtons == 0) {
            interruptHandler->requestJoypadInterrupt();
        }
    }
}

void GbJoypad::processKeyReleaseEvent(int key)
{
    std::lock_guard lock(mutex);
    if (key == Qt::Key_S) {
        directionButtionState.down = 1;
    } else if (key == Qt::Key_W) {
        directionButtionState.up = 1;
    } else if (key == Qt::Key_A) {
        directionButtionState.left = 1;
    } else if (key == Qt::Key_D) {
        directionButtionState.right = 1;
    } else  if (key == Qt::Key_3) {
        actionButtonsState.start = 1;
    } else if (key == Qt::Key_2) {
        actionButtonsState.select = 1;
    } else if (key == Qt::Key_K) {
        actionButtonsState.b = 1;
    } else if (key == Qt::Key_J) {
        actionButtonsState.a = 1;
    }
}

}  // namespace qnemu
