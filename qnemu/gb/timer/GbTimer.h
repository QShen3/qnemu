/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

/**
 * FF04-FF07 timer/divider.
 *
 * The 16-bit system counter increments every T-cycle (twice per T-cycle in
 * CGB double speed).  TIMA increments on the falling edge of the selected
 * counter bit (bits 9, 3, 5 or 7).  Overflow is visible immediately as TIMA
 * = 0, but TMA is reloaded and the interrupt requested one M-cycle later.
 */
class GbTimer : public GbDeviceInterface
{
public:
    GbTimer() = delete;
    explicit GbTimer(std::shared_ptr<GbInterruptHandlerInterface> interruptHandler);
    ~GbTimer() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    void setDoubleSpeed(bool value);

    /// Override the internal divider, used to reproduce the post-boot DIV
    /// phase when the test runner does not execute a boot ROM.
    void setDivider(uint16_t value);

    /// Consume a pending DIV-APU clock (falling edge of DIV bit 4, or bit 5
    /// in double speed).  The APU remains at 512 Hz in both speed modes.
    bool takeDivApuTick();

    bool isDivApuBitHigh() const;

private:
    uint8_t selectedClockBitIndex() const;
    uint8_t selectedClockBit() const;
    uint8_t divApuBitIndex() const;
    void tick();
    void incrementTimerCounter();
    void updateDivApuBit();

    uint16_t divider = 0xAB00;
    uint8_t timerCounter = 0;
    uint8_t timerModulo = 0;
    uint8_t timerControl = 0xF8;  // bits 2-0 meaningful; upper bits read 1

    bool doubleSpeed = false;
    bool overflow = false;           // TIMA just overflowed; reload pending
    uint8_t overflowCycles = 0;      // T-cycles since the overflow
    uint8_t reloadWindowCycles = 0;  // TMA still wins CPU writes this M-cycle
    uint8_t previousTimerBit = 0;

    uint8_t previousDivApuBit = 0;
    bool divApuTickPending = false;

    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
};

}  // namespace qnemu
