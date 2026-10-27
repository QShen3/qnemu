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

class GbTimer : public GbDeviceInterface
{
public:
    GbTimer() = delete;
    explicit GbTimer(std::shared_ptr<GbInterruptHandlerInterface> interruptHander);
    ~GbTimer() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    /// Consume a pending DIV-APU clock (falling edge of DIV bit 4).
    bool takeDivApuTick();

    bool isDivApuBitHigh() const
    {
        return ((registers.divider >> 12) & 0x01) != 0;
    }

private:
    void updateDivApuBit();
    struct {
        uint16_t divider;
        uint8_t timerCounter;
        uint8_t timerModulo;
        union  {
            struct {
                uint8_t inputClockSelect : 2;
                uint8_t timerEnable : 1;
                uint8_t : 5;
            };
            uint8_t timerControl;
        };
    } registers{};
    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    bool overflow;
    uint8_t previousBit;
    uint8_t ticksSinceOverflow;
    uint8_t previousDivApuBit;
    bool divApuTickPending;
};

}  // namespace qnemu
