/*
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

/**
 * Length timer shared by all four APU channels.
 *
 * The register value is an "initial length": the hardware stores the
 * complement of the written value, and counts it down to zero.  A written
 * value of 0 therefore means the maximum length (64 for CH1/2/4, 256 for CH3).
 */
class GbLengthTimer
{
public:
    explicit GbLengthTimer(uint16_t maxLength = 64);
    ~GbLengthTimer() = default;

    void reset();
    void setLength(uint8_t registerValue);
    void setCounter(uint16_t value) { length = value; }
    void setEnabled(bool enabled);
    void trigger();
    void clock();

    bool isEnabled() const { return enabled; }
    bool expired() const { return length == 0; }
    uint16_t getLength() const { return length; }

private:
    uint16_t maxLength;
    uint16_t length = 0;
    bool enabled = false;
};

}  // namespace qnemu
