/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"

namespace qnemu
{

class GbChannel1 : public GbChannelInterface
{
public:
    GbChannel1() = default;
    ~GbChannel1() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t getData() const override;

private:
    struct {
        union {
            struct {
                uint8_t channel1IndividualStep : 3;
                uint8_t channel1Direction : 1;
                uint8_t channel1Pace : 3;
                uint8_t : 1;
            };
            uint8_t channel1Sweep;
        };  // FF10
        union {
            struct {
                uint8_t channel1InitialLengthTimer : 6;
                uint8_t channel1WaveDuty : 2;
            };
            uint8_t channel1LengthTimerAndDutyCycle;
        };  // FF11
        union {
            struct {
                uint8_t channel1SweepPace : 3;
                uint8_t channel1EnvelopeDirection : 1;
                uint8_t channel1InitialVolume : 4;
            };
            uint8_t channel1VolumeAndEnvelope;
        };  // FF12
        uint8_t channel1PeriodLow; // FF13
        union {
            struct {
                uint8_t channel1PeriodHigh : 3;
                uint8_t : 3;
                uint8_t channel1LengthEnable : 1;
                uint8_t channel1Trigger : 1;
            };
            uint8_t channel1PeriodHighAndControl;
        };  // FF14
    } registers;

    uint8_t data;

};

}  // namespace qnemu