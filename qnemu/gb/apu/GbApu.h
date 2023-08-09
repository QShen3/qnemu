/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemu
{

class GbApu : public GbDeviceInterface
{
public:
    GbApu() = default;
    ~GbApu() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    struct {
        union {
            struct {
                uint8_t channel1SweepSlopeControl : 3;
                uint8_t channel1SweepIncreaseOrDecrease : 1;
                uint8_t channel1SweepPace : 3;
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
                uint8_t channel1InitialVolumeOfEnvelope : 4;
            };
            uint8_t channel1VolumeAndEnvelope;
        };  // FF12
        uint8_t channel1PeriodLow; // FF13
        union {
            struct {
                uint8_t channel1PeriodHigh : 3;
                uint8_t : 3;
                uint8_t channel1SoundLenghtEnable : 1;
                uint8_t channel1Trigger : 1;
            };
            uint8_t channel1PeriodHighAndControl;
        };  // FF14
        union {
            struct {
                uint8_t channel2InitialLengthTimer : 6;
                uint8_t channel2WaveDuty : 2;
            };
            uint8_t channel2LengthTimerAndDutyCycle;
        };  // FF16
        union {
            struct {
                uint8_t channel2SweepPace : 3;
                uint8_t channel2EnvelopeDirection : 1;
                uint8_t channel2InitialVolumeOfEnvelope : 4;
            };
            uint8_t channel2VolumeAndEnvelope;
        };  // FF17
        uint8_t channel2PeriodLow; // FF18
        union {
            struct {
                uint8_t channel2PeriodHigh : 3;
                uint8_t : 3;
                uint8_t channel2SoundLenghtEnable : 1;
                uint8_t channel2Trigger : 1;
            };
            uint8_t channel2PeriodHighAndControl;
        };  // FF19
        union {
            struct {
                uint8_t : 7;
                uint8_t channel3Dac : 1;
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
                uint8_t channel3SoundLenghtEnable : 1;
                uint8_t channel3Trigger : 1;
            };
            uint8_t channel3PeriodHighAndControl;
        };  // FF1E
    } registers;
};

}  // namespace qnemu
