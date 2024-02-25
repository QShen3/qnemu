/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

class GbApu : public GbDeviceInterface
{
public:
    GbApu(std::unique_ptr<SoundInterface> sound, std::array<std::unique_ptr<GbChannelInterface>, 4> channels);
    ~GbApu();

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

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
                uint8_t channel2InitialVolume : 4;
            };
            uint8_t channel2VolumeAndEnvelope;
        };  // FF17
        uint8_t channel2PeriodLow; // FF18
        union {
            struct {
                uint8_t channel2PeriodHigh : 3;
                uint8_t : 3;
                uint8_t channel2LengthEnable : 1;
                uint8_t channel2Trigger : 1;
            };
            uint8_t channel2PeriodHighAndControl;
        };  // FF19
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
        uint8_t channel4LengthTimer;  // FF20
        union {
            struct {
                uint8_t channel4SweepPace : 3;
                uint8_t channel4EnvelopeDirection : 1;
                uint8_t channel4InitialVolume : 4;
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
                uint8_t channel4LengthEnable : 1;
                uint8_t channel4Trigger : 4;
            };
            uint8_t channel4Control;
        };  // FF23
        union {
            struct {
                uint8_t rightVolume : 3;
                uint8_t vinRight : 1;
                uint8_t leftVolume : 3;
                uint8_t vinLeft : 1;
            };
            uint8_t masterVolumeAndVinPanning;
        };  // FF24
        union {
            struct {
                uint8_t channel1Right : 1;
                uint8_t channel2Right : 1;
                uint8_t channel3Right : 1;
                uint8_t channel4Right : 1;
                uint8_t channel1Left : 1;
                uint8_t channel2Left : 1;
                uint8_t channel3Left : 1;
                uint8_t channel4Left : 1;
            };
            uint8_t soundPanning;
        };  // FF25
        union {
            struct {
                uint8_t channel1On : 1;
                uint8_t channel2On : 1;
                uint8_t channel3On : 1;
                uint8_t channel4On : 1;
                uint8_t : 3;
                uint8_t audioOn : 1;
            };
            uint8_t audioMasterControl;
        };  // FF26
    } registers;

    std::unique_ptr<SoundInterface> sound;
    // std::array<uint8_t, 0xF> wavePatternRam;
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels;
};

}  // namespace qnemu
