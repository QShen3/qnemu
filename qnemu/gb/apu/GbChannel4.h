/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"

namespace qnemu
{

class GbChannel4 : public GbChannelInterface
{
public:
    GbChannel4() = default;
    ~GbChannel4() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t getData() const override;

private:
    struct {
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
    } registers;

    uint8_t data;
};

}  // namespace qnemu