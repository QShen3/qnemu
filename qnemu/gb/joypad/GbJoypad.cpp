/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/joypad/GbJoypad.h"

namespace qnemu
{

GbJoypad::GbJoypad(std::shared_ptr<GbInterruptHandlerInterface> interruptHandler)
    : interruptHandler(std::move(interruptHandler))
{
    GbJoypad::reset();
}

uint8_t GbJoypad::read(uint16_t address) const
{
    if (address != 0xFF00) {
        assert(false && "Wrong address");
        return 0xFF;
    }

    const std::lock_guard lock(mutex);
    // Bits 6-7 are unused and read as 1.  The low nibble is the AND of every
    // selected group (0xF when neither group is selected).
    return static_cast<uint8_t>(0xC0 | registers.select | selectedLowNibble());
}

void GbJoypad::write(uint16_t address, const uint8_t& value)
{
    if (address != 0xFF00) {
        assert(false && "Wrong address");
        return;
    }

    bool requestInterrupt = false;
    {
        const std::lock_guard lock(mutex);
        const uint8_t before = selectedLowNibble();
        registers.select = static_cast<uint8_t>(value & 0x30);
        const uint8_t after = selectedLowNibble();

        // Selecting a group while one of its buttons is held creates a
        // high-to-low edge on that input line.
        requestInterrupt = (before & ~after & 0x0F) != 0;
    }

    if (requestInterrupt) {
        interruptHandler->requestJoypadInterrupt();
    }
}

void GbJoypad::step()
{
}

void GbJoypad::reset()
{
    const std::lock_guard lock(mutex);
    registers.select = 0x00;  // both groups selected (P1 reads 0xCF)
    actionButtons = 0x0F;
    directionButtons = 0x0F;
}

void GbJoypad::setButtonState(GbButton button, bool pressed)
{
    bool requestInterrupt = false;
    {
        const std::lock_guard lock(mutex);
        const uint8_t before = selectedLowNibble();

        const bool isActionButton = static_cast<uint8_t>(button)
            >= static_cast<uint8_t>(GbButton::a);
        uint8_t& group = isActionButton ? actionButtons : directionButtons;
        const uint8_t mask = maskForButton(button);

        if (pressed) {
            group = static_cast<uint8_t>(group & ~mask);
        } else {
            group = static_cast<uint8_t>(group | mask);
        }

        const uint8_t after = selectedLowNibble();
        requestInterrupt = (before & ~after & 0x0F) != 0;
    }

    if (requestInterrupt) {
        interruptHandler->requestJoypadInterrupt();
    }
}

uint8_t GbJoypad::selectedLowNibble() const
{
    uint8_t low = 0x0F;
    if ((registers.select & 0x10) == 0) {
        low = static_cast<uint8_t>(low & directionButtons);
    }
    if ((registers.select & 0x20) == 0) {
        low = static_cast<uint8_t>(low & actionButtons);
    }
    return low;
}

uint8_t GbJoypad::maskForButton(GbButton button)
{
    switch (button) {
    case GbButton::right:
    case GbButton::a:
        return 0x01;
    case GbButton::left:
    case GbButton::b:
        return 0x02;
    case GbButton::up:
    case GbButton::select:
        return 0x04;
    case GbButton::down:
    case GbButton::start:
        return 0x08;
    }
    return 0x00;
}

}  // namespace qnemu
