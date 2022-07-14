/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemu
{

class GbTimer : public GbDeviceInterface
{
public:
    GbTimer() = delete;
    GbTimer(std::shared_ptr<GbInterruptHandler> interruptHander);
    ~GbTimer() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    struct {
        uint8_t divider;
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
    } registers;
    std::shared_ptr<GbInterruptHandler> interruptHandler;
};

}  // namespace qnemu
