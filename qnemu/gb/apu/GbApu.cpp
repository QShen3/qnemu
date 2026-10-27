/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

GbApu::GbApu(std::unique_ptr<SoundInterface> sound,
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels) :
    sound(std::move(sound)),
    channels(std::move(channels))
{
    reset();
}

uint8_t GbApu::read(uint16_t address) const
{
    // Wave RAM is accessible even while the APU is powered down.
    if (address >= 0xFF30 && address <= 0xFF3F) {
        return channels[2]->read(address);
    }

    if (address == 0xFF26) {
        uint8_t status = 0;
        if (powered) {
            for (std::size_t i = 0; i < channels.size(); ++i) {
                if (channels[i]->isEnabled()) {
                    status |= static_cast<uint8_t>(1U << i);
                }
            }
        }
        return static_cast<uint8_t>(0x70 | (powered ? 0x80 : 0x00) | status);
    }

    if (!powered) {
        if (address >= 0xFF10 && address <= 0xFF25) {
            return 0x00;
        }
        return 0xFF;
    }

    if (address >= 0xFF10 && address <= 0xFF14) {
        return channels[0]->read(address);
    }
    if (address >= 0xFF16 && address <= 0xFF19) {
        return channels[1]->read(address);
    }
    if (address >= 0xFF1A && address <= 0xFF1E) {
        return channels[2]->read(address);
    }
    if (address >= 0xFF20 && address <= 0xFF23) {
        return channels[3]->read(address);
    }
    if (address == 0xFF24) {
        return nr50;
    }
    if (address == 0xFF25) {
        return nr51;
    }

    return 0xFF;
}

void GbApu::write(uint16_t address, const uint8_t& value)
{
    // Wave RAM is accessible even while the APU is powered down.  CH3 handles
    // the "CPU sees the current byte" behaviour while it is playing.
    if (address >= 0xFF30 && address <= 0xFF3F) {
        channels[2]->write(address, value);
        return;
    }

    if (address == 0xFF26) {
        const bool turnOn = (value & 0x80) != 0;
        if (!turnOn) {
            if (powered) {
                powered = false;
                for (auto& channel : channels) {
                    channel->powerOff();
                }

                nr50 = 0;
                nr51 = 0;
                frameCounter = 0;
                frameStep = 0;
                sampleAccumulator = 0;
                sampleCycles = 0;
                mixAccumLeft = 0;
                mixAccumRight = 0;
                mixLeft = 0;
                mixRight = 0;
                dacsEnabled = false;
                capacitorLeft = 0;
                capacitorRight = 0;
                sampleCount = 0;
            }
        } else {
            powered = true;
        }
        return;
    }

    if (!powered) {
        return;
    }

    if (address >= 0xFF10 && address <= 0xFF14) {
        channels[0]->write(address, value);
    } else if (address >= 0xFF16 && address <= 0xFF19) {
        channels[1]->write(address, value);
    } else if (address >= 0xFF1A && address <= 0xFF1E) {
        channels[2]->write(address, value);
    } else if (address >= 0xFF20 && address <= 0xFF23) {
        channels[3]->write(address, value);
    } else if (address == 0xFF24) {
        nr50 = value;
    } else if (address == 0xFF25) {
        nr51 = value;
    }
}

void GbApu::clockFrameSequencer()
{
    // 512 Hz frame sequencer: length 256 Hz, sweep 128 Hz, envelope 64 Hz.
    if ((frameStep & 0x01) == 0) {
        for (auto& channel : channels) {
            channel->clockLength();
        }
    }
    if (frameStep == 2 || frameStep == 6) {
        for (auto& channel : channels) {
            channel->clockSweep();
        }
    }
    if (frameStep == 7) {
        for (auto& channel : channels) {
            channel->clockEnvelope();
        }
    }

    frameStep = static_cast<uint8_t>((frameStep + 1) & 0x07);
}

void GbApu::step()
{
    if (!powered) {
        return;
    }

    for (auto& channel : channels) {
        channel->step();
    }

    ++frameCounter;
    if (frameCounter >= FrameSequencerPeriod) {
        frameCounter = 0;
        clockFrameSequencer();
    }

    mixAndAccumulate();

    ++sampleCycles;
    sampleAccumulator += SampleRate;
    if (sampleAccumulator >= CpuFrequency) {
        sampleAccumulator -= CpuFrequency;
        emitSample();
    }
}

void GbApu::mixAndAccumulate()
{
    int32_t left = 0;
    int32_t right = 0;
    bool anyDac = false;

    for (std::size_t i = 0; i < channels.size(); ++i) {
        const auto& channel = *channels[i];
        if (!channel.isDacEnabled()) {
            continue;
        }

        anyDac = true;

        // The DAC has a negative slope: digital 0 is the most positive level
        // and digital 15 the most negative one.
        const int32_t analog = 15 - 2 * static_cast<int32_t>(channel.getData());
        if ((nr51 & (0x10U << i)) != 0) {
            left += analog;
        }
        if ((nr51 & (1U << i)) != 0) {
            right += analog;
        }
    }

    mixLeft = left * ((nr50 & 0x07) + 1);
    mixRight = right * (((nr50 >> 4) & 0x07) + 1);
    dacsEnabled = anyDac;

    mixAccumLeft += mixLeft;
    mixAccumRight += mixRight;
}

void GbApu::emitSample()
{
    if (sampleCycles == 0) {
        return;
    }

    const int32_t left = static_cast<int32_t>(
        mixAccumLeft / static_cast<int64_t>(sampleCycles));
    const int32_t right = static_cast<int32_t>(
        mixAccumRight / static_cast<int64_t>(sampleCycles));

    mixAccumLeft = 0;
    mixAccumRight = 0;
    sampleCycles = 0;

    double outputLeft = 0;
    double outputRight = 0;
    if (dacsEnabled) {
        // High-pass filter: removes the DC bias from enabled DACs and
        // off-centre duty cycles, just like the hardware's output capacitor.
        outputLeft = static_cast<double>(left) - capacitorLeft;
        capacitorLeft = static_cast<double>(left) - outputLeft * HighPassCharge;
        outputRight = static_cast<double>(right) - capacitorRight;
        capacitorRight = static_cast<double>(right) - outputRight * HighPassCharge;
    }

    constexpr double MaxMix = 4.0 * 15.0 * 8.0;  // 4 channels, ±15, volume 8
    const auto toInt16 = [](double value) -> int16_t {
        const double scaled = value * 32767.0 / MaxMix;
        return static_cast<int16_t>(std::clamp(scaled, -32768.0, 32767.0));
    };

    sampleBuffer[sampleCount++] = toInt16(outputLeft);
    sampleBuffer[sampleCount++] = toInt16(outputRight);

    if (sampleCount == sampleBuffer.size()) {
        flushSamples();
    }
}

void GbApu::flushSamples()
{
    if (sound && sampleCount > 0) {
        sound->addSamples(sampleBuffer.data(), sampleCount / 2);
    }
    sampleCount = 0;
}

void GbApu::reset()
{
    powered = true;
    nr50 = 0x77;
    nr51 = 0xF3;

    for (auto& channel : channels) {
        channel->reset();
    }

    frameCounter = 0;
    frameStep = 0;
    sampleAccumulator = 0;
    sampleCycles = 0;
    mixAccumLeft = 0;
    mixAccumRight = 0;
    mixLeft = 0;
    mixRight = 0;
    dacsEnabled = false;
    capacitorLeft = 0;
    capacitorRight = 0;
    sampleCount = 0;
}

}  // namespace qnemu
