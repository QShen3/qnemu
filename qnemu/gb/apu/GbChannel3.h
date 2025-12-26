/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"

namespace qnemu
{

class GbChannel3 : public GbChannelInterface
{
public:
    GbChannel3();
    ~GbChannel3() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t getData() const override;

private:
    struct {
        union {
            struct {
                uint8_t : 7;
                uint8_t channel3DacOn : 1;
            };
            uint8_t channel3DacEnable;
        };  // FF1A
        uint8_t channel3LengthTimer;  // FF1B
        union {
            struct {
                uint8_t : 5;
                uint8_t channel3OutputLevelSelection : 2;
                uint8_t : 1;
            };
            uint8_t channel3OutputLevel;
        };  // FF1C
        uint8_t channel3PeriodLow; // FF1D
        union {
            struct {
                uint8_t channel3PeriodHigh : 3;
                uint8_t : 3;
                uint8_t channel3LengthEnable : 1;
                uint8_t channel3Trigger : 1;
            };
            uint8_t channel3PeriodHighAndControl;
        };  // FF1E
    } registers;

    uint8_t data;
};

}  // namespace qnemu