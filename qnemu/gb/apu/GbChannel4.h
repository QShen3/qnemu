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

/**
 * Channel 4: pseudo-random noise from a 15-bit (or 7-bit) LFSR.
 *
 * Registers: NR41 (length), NR42 (volume + envelope), NR43 (clock divider
 * and LFSR width), NR44 (control).
 */
class GbChannel4 : public GbChannelInterface
{
public:
    GbChannel4();
    ~GbChannel4() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    uint8_t getData() const override { return data; }
    bool isEnabled() const override { return enabled; }
    bool isDacEnabled() const override { return envelope.isDacEnabled(); }

    void clockLength() override;
    void clockEnvelope() override;

    /// Current LFSR state (exposed for tests and diagnostics).
    uint16_t getLfsr() const { return lfsr; }

private:
    void trigger();
    uint32_t clockPeriod() const;
    void stepLfsr();

    uint8_t nr41 = 0;  // FF20
    uint8_t nr42 = 0;  // FF21
    uint8_t nr43 = 0;  // FF22
    uint8_t nr44 = 0;  // FF23

    bool enabled = false;
    uint32_t timer = 0;
    uint16_t lfsr = 0;
    uint8_t data = 0;

    GbLengthTimer lengthTimer{64};
    GbVolumeEnvelope envelope;
};

}  // namespace qnemu
