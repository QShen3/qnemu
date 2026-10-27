/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/joypad/GbButton.h"

namespace qnemu
{

/**
 * FF00 / P1 (JOYP) joypad matrix.
 *
 * Bits 4 and 5 select the d-pad and action-button groups (active low).  The
 * low nibble is the AND of every selected group, and reads 0xF when neither
 * group is selected.  Bits 6 and 7 always read 1 and ignore writes.
 *
 * The joypad interrupt is edge-triggered: it is requested when a selected
 * input line goes from high to low, either because a button was pressed or
 * because the selection changed while a button was already held.
 */
class GbJoypad : public GbDeviceInterface
{
public:
    GbJoypad() = delete;
    explicit GbJoypad(std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~GbJoypad() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    /// Called by the host input backend.  Safe to call from another thread.
    void setButtonState(GbButton button, bool pressed);

private:
    uint8_t selectedLowNibble() const;
    static uint8_t maskForButton(GbButton button);

    struct {
        uint8_t select = 0x00;  // bits 4-5; 0 means both groups selected
    } registers;

    uint8_t actionButtons = 0x0F;     // active low: A, B, Select, Start
    uint8_t directionButtons = 0x0F;  // active low: Right, Left, Up, Down

    mutable std::mutex mutex;
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
};

}  // namespace qnemu
