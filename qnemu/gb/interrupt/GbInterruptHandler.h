/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbGpu;

class GbInterruptHandler : public GbDeviceInterface
{
public:
    GbInterruptHandler() = delete;
    explicit GbInterruptHandler(std::shared_ptr<GbCpuInterface> cpu);
    ~GbInterruptHandler() = default;

    bool accepts(uint16_t address) const override;
    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    friend GbGpu;
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
    std::weak_ptr<GbCpuInterface> cpu;
};

}  // namespace qnemu