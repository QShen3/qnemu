/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>

#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/apu/GbLengthTimer.h"

namespace qnemu
{

/**
 * Channel 3: 32-sample, 4-bit wave-table channel.
 *
 * Registers: NR30 (DAC), NR31 (length), NR32 (output level), NR33/NR34
 * (period + control), and FF30..FF3F (wave RAM).
 */
class GbChannel3 : public GbChannelInterface
{
public:
    explicit GbChannel3(bool cgb = true);
    ~GbChannel3() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;
    void powerOff(bool preserveLength = false) override;

    uint8_t getData() const override { return data; }
    bool isEnabled() const override { return enabled; }
    bool isDacEnabled() const override { return (nr30 & 0x80) != 0; }

    void clockLength() override;
    void setLengthClockPhase(bool nextStepClocksLength) override
    {
        this->nextStepClocksLength = nextStepClocksLength;
    }
    void setCgb(bool cgb) override { this->cgb = cgb; }
    void writeLengthWhileOff(uint8_t value) override { lengthTimer.setLength(value); }

    /// Current 11-bit period value (exposed for tests and diagnostics).
    uint16_t getPeriod() const { return period(); }

private:
    uint16_t period() const
    {
        return static_cast<uint16_t>(((nr34 & 0x07) << 8) | nr33);
    }
    void trigger();
    uint8_t applyOutputLevel(uint8_t sample) const;
    uint8_t readWaveRam(uint16_t address) const;
    void writeWaveRam(uint16_t address, uint8_t value);

    uint8_t nr30 = 0;  // FF1A
    uint8_t nr31 = 0;  // FF1B
    uint8_t nr32 = 0;  // FF1C
    uint8_t nr33 = 0;  // FF1D
    uint8_t nr34 = 0;  // FF1E

    bool cgb = true;
    bool enabled = false;
    bool nextStepClocksLength = true;
    uint16_t frequencyTimer = 0;
    uint8_t wavePosition = 0;  // 0..31, index of the most recent sample
    uint8_t sampleBuffer = 0;
    uint8_t waveReadWindow = 0;
    uint8_t data = 0;

    GbLengthTimer lengthTimer{256};
    std::array<uint8_t, 16> waveRam{};
};

}  // namespace qnemu
