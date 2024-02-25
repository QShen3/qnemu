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
    ~GbApu() = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

private:
    struct {
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
