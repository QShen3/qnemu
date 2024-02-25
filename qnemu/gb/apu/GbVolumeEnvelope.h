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
    GbVolumeEnvelope() = default;
    ~GbVolumeEnvelope() = default;

    void step();
    uint8_t getVolume() const;
};

} // namespace qnemu
