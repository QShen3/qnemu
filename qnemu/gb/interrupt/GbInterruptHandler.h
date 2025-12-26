/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemu
{

class GbGpu;
class GbJoypad;
class GbTimer;

class GbInterruptHandler : public GbInterruptHandlerInterface
{
public:
    GbInterruptHandler();
    ~GbInterruptHandler() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    void registerCpuCallback(std::function<void(GbInterrupt)>) override;

    void requestVBlankInterrupt() override;
    void requestLcdInterrupt() override;
    void requestTimerInterrupt() override;
    void requestSerialInterrupt() override;
    void requestJoypadInterrupt() override;

private:
    struct {
        union {
            struct {
                uint8_t vBlankRequest : 1;
                uint8_t lcdRequest : 1;
                uint8_t timerRequest : 1;
                uint8_t serialRequest : 1;
                uint8_t joyPadRequest : 1;
                uint8_t interruptFlagReserved : 3;
            };
            uint8_t interruptFlag;
        };
        union {
            struct {
                uint8_t vBlankEnabled : 1;
                uint8_t lcdEnabled : 1;
                uint8_t timerEnabled : 1;
                uint8_t serialEnabled : 1;
                uint8_t joyPadEnabled : 1;
                uint8_t interruptEnabledReserved : 3;
            };
            uint8_t interruptEnabled;
        };
    } registers;
    std::function<void(GbInterrupt)> cpuCallback;
};

}  // namespace qnemu