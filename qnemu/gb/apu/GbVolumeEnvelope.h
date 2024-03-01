/* 
 *  Copyright [2024] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

namespace qnemu
{

class GbVolumeEnvelope
{
public:
    GbVolumeEnvelope();
    ~GbVolumeEnvelope() = default;

    void step();
    void setInitialVolume(uint8_t volume);
    void setDirection(bool direction);
    void setPace(uint8_t pace);
    void reset();
    void trigger();

    uint8_t getVolume() const;

private:
    uint8_t initialVolume;
    bool direction;
    uint8_t pace;
    bool finished;
    uint8_t volume;
    uint16_t counter;
};

} // namespace qnemu
