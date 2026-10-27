/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <utility>

#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

void GbApu::Biquad::configure(double cutoff, double sampleRate)
{
    const double omega = 2.0 * std::numbers::pi * cutoff / sampleRate;
    const double cosine = std::cos(omega);
    constexpr double Q = 0.7071067811865476;  // Butterworth
    const double alpha = std::sin(omega) / (2.0 * Q);
    const double a0 = 1.0 + alpha;

    b0 = (1.0 - cosine) * 0.5 / a0;
    b1 = (1.0 - cosine) / a0;
    b2 = b0;
    a1 = -2.0 * cosine / a0;
    a2 = (1.0 - alpha) / a0;
    reset();
}

double GbApu::Biquad::process(double input)
{
    const double output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    return output;
}

void GbApu::Biquad::reset()
{
    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
}

GbApu::GbApu(std::unique_ptr<SoundInterface> sound,
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels,
    bool cgb,
    uint32_t fadeInFrames) :
    sound(std::move(sound)),
    channels(std::move(channels)),
    cgb(cgb),
    fadeInFrames(fadeInFrames)
{
    lowPassLeft.configure(LowPassCutoff, SampleRate);
    lowPassRight.configure(LowPassCutoff, SampleRate);
    this->channels[2]->setCgb(cgb);
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
                    // DMG keeps the length counters across APU power-off.
                    channel->powerOff(!cgb);
                }

                nr50 = 0;
                nr51 = 0;
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
        } else if (!powered) {
            powered = true;
            // Powering up restarts the frame sequencer phase.
            frameStep = 0;
            fadeInRemaining = fadeInFrames;
        }
        return;
    }

    if (!powered) {
        // On DMG the length registers (NRx1) are not cleared by power-off and
        // remain writable; all other APU registers are read-only while off.
        if (!cgb) {
            switch (address) {
            case 0xFF11:
                channels[0]->writeLengthWhileOff(value);
                break;
            case 0xFF16:
                channels[1]->writeLengthWhileOff(value);
                break;
            case 0xFF1B:
                channels[2]->writeLengthWhileOff(value);
                break;
            case 0xFF20:
                channels[3]->writeLengthWhileOff(value);
                break;
            default:
                break;
            }
        }
        return;
    }

    const bool nextStepClocksLength = (frameStep & 0x01) == 0;
    if (address >= 0xFF10 && address <= 0xFF14) {
        channels[0]->setLengthClockPhase(nextStepClocksLength);
        channels[0]->write(address, value);
    } else if (address >= 0xFF16 && address <= 0xFF19) {
        channels[1]->setLengthClockPhase(nextStepClocksLength);
        channels[1]->write(address, value);
    } else if (address >= 0xFF1A && address <= 0xFF1E) {
        channels[2]->setLengthClockPhase(nextStepClocksLength);
        channels[2]->write(address, value);
    } else if (address >= 0xFF20 && address <= 0xFF23) {
        channels[3]->setLengthClockPhase(nextStepClocksLength);
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

void GbApu::divApuTick()
{
    if (skipDivApuEvent) {
        skipDivApuEvent = false;
        return;
    }

    if (powered) {
        clockFrameSequencer();
    } else {
        // The DIV-APU counter keeps running while the APU is off; only the
        // channel state machines are cleared/stopped.
        frameStep = static_cast<uint8_t>((frameStep + 1) & 0x07);
    }
}

void GbApu::step()
{
    if (!powered) {
        return;
    }

    for (auto& channel : channels) {
        channel->step();
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

    if (anyDac && !hadDacs) {
        // Turning all DACs back on creates a DC step.  Fade the output in
        // over a few milliseconds so it does not become an audible click.
        fadeInRemaining = fadeInFrames;
        lowPassLeft.reset();
        lowPassRight.reset();
    }
    hadDacs = anyDac;

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

        // Gentle low-pass to soften the square-wave edges and reduce the
        // harshness of the raw DAC output.
        outputLeft = lowPassLeft.process(outputLeft);
        outputRight = lowPassRight.process(outputRight);
    }

    if (fadeInFrames > 0 && fadeInRemaining > 0) {
        const double gain = 1.0
            - static_cast<double>(fadeInRemaining) / static_cast<double>(fadeInFrames);
        outputLeft *= gain;
        outputRight *= gain;
        --fadeInRemaining;
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

    frameStep = 0;
    skipDivApuEvent = false;
    fadeInRemaining = fadeInFrames;
    hadDacs = false;
    lowPassLeft.reset();
    lowPassRight.reset();
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

    if (sound) {
        sound->flush();
    }
}

}  // namespace qnemu
