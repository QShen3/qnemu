/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

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
                uint8_t channel1SoundLengthEnable : 1;
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
                uint8_t channel2SoundLengthEnable : 1;
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
                uint8_t channel3SoundLengthEnable : 1;
                uint8_t channel3Trigger : 1;
            };
            uint8_t channel3PeriodHighAndControl;
        };  // FF1E
        uint8_t channel4LengthTimer;  // FF20
        union {
            struct {
                uint8_t channel4SweepPace : 3;
                uint8_t channel4EnvelopeDirection : 1;
                uint8_t channel4InitialVolumeOfEnvelope : 4;
            };
            uint8_t channel4VolumeAndEnvelope;
        };  // FF21
        union {
            struct {
                uint8_t channel4ClockDivider : 3;
                uint8_t channel4LfsrWidth : 1;
                uint8_t channel4ClockShift : 4;
            };
            uint8_t channel4FrequencyAndRandomness;
        };  // FF22
        union {
            struct {
                uint8_t : 6;
                uint8_t channel4SoundLengthEnable : 1;
                uint8_t channel4Trigger : 4;
            };
            uint8_t channel4Control;
        };  // FF23
        union {
            struct {
                uint8_t rightOutputVolume : 3;
                uint8_t mixVinIntoRightOutput : 1;
                uint8_t LeftOutputVolume : 3;
                uint8_t mixVinIntoLeftOutput : 1;
            };
            uint8_t masterVolumeAndVinPanning;
        };  // FF24
        union {
            struct {
                uint8_t mixChannel1IntoRightOutput : 1;
                uint8_t mixChannel2IntoRightOutput : 1;
                uint8_t mixChannel3IntoRightOutput : 1;
                uint8_t mixChannel4IntoRightOutput : 1;
                uint8_t mixChannel1IntoLeftOutput : 1;
                uint8_t mixChannel2IntoLeftOutput : 1;
                uint8_t mixChannel3IntoLeftOutput : 1;
                uint8_t mixChannel4IntoLeftOutput : 1;
            };
            uint8_t soundPanning;
        };  // FF25
        union {
            struct {
                uint8_t channel1OnFlag : 1;
                uint8_t channel2OnFlag : 1;
                uint8_t channel3OnFlag : 1;
                uint8_t channel4OnFlag : 1;
                uint8_t : 3;
                uint8_t allSoundOnOrOff : 1;
            };
            uint8_t soundOnOrOff;
        };  // FF26
    } registers;
    std::array<uint8_t, 0xF> wavePatternRam;
};

}  // namespace qnemu
