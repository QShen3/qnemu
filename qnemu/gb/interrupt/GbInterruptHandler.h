/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

class GbInterruptHandler : public GbInterruptHandlerInterface
{
public:
    GbInterruptHandler();
    ~GbInterruptHandler() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    void request(GbInterrupt interrupt) override;
    uint8_t pending() const override;
    void acknowledge(uint8_t mask) override;

    void requestVBlankInterrupt() override;
    void requestLcdInterrupt() override;
    void requestTimerInterrupt() override;
    void requestSerialInterrupt() override;
    void requestJoypadInterrupt() override;

private:
    uint8_t interruptFlag = 0xE1;
    uint8_t interruptEnabled = 0x00;
};

}  // namespace qnemu
