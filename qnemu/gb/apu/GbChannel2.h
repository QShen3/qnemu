/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/apu/GbLengthTimer.h"
#include "qnemu/gb/apu/GbVolumeEnvelope.h"

namespace qnemu
{

class GbChannel2 : public GbChannelInterface
{
public:
    GbChannel2();
    ~GbChannel2() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t getData() const override;

private:
    struct {
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
    } registers;

    uint8_t data;
    bool enabled;
    GbLengthTimer lengthTimer;
    GbVolumeEnvelope volumeEnvelope;
};

}  // namespace qnemu