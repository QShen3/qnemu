/* 
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

class GbLengthTimer
{
public:
    GbLengthTimer();
    ~GbLengthTimer() = default;

    void step();
    void setLength(uint8_t length);
    void enable();
    void disable();
    void reset();

    uint8_t getLength() const;
    bool isEnabled() const;

private:
    uint8_t length;
    uint16_t counter;
    bool enabled;
};

}  // namespace qnemu
