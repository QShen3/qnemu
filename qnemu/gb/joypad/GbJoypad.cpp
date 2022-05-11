/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <memory>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/gb/joypad/GbJoypad.h"

namespace qnemu
{

GbJoypad::GbJoypad(std::shared_ptr<DisplayInterface> display)
    : display(display)
{
    GbJoypad::reset();
}

bool GbJoypad::accepts(uint16_t address) const
{
    return address == 0xFF00;
}

uint8_t GbJoypad::read(uint16_t address) const
{
    if (address == 0xFF00) {
        return registers.joypadState;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void GbJoypad::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF00) {
        registers.joypadState = value & 0b00110000;
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

}  // namespace qnemu
