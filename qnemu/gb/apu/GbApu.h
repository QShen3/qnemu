/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

/**
 * Game Boy Audio Processing Unit.
 *
 * The APU owns the four channels and a frame sequencer clocked at 512 Hz.
 * `step()` is called once per T-cycle.  The mixer converts the channels'
 * 4-bit outputs to analog levels, applies NR50/NR51, then a DC-blocking
 * high-pass filter and a box filter downsample to 44100 Hz stereo int16.
 */
class GbApu : public GbDeviceInterface
{
public:
    static constexpr uint32_t SampleRate = 44100;

    /// Fade-in used to suppress the DAC-enable click.  20 ms at 44.1 kHz.
    static constexpr uint32_t DefaultFadeInFrames = SampleRate / 50;

    GbApu(std::unique_ptr<SoundInterface> sound,
        std::array<std::unique_ptr<GbChannelInterface>, 4> channels,
        bool cgb = true,
        uint32_t fadeInFrames = 0);
    ~GbApu() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t& value) override;
    void step() override;
    void reset() override;

    /// One DIV-APU tick (512 Hz, on the falling edge of DIV bit 4).  The frame
    /// sequencer runs from this rather than from an internal cycle counter so
    /// that programs can synchronise to it by writing DIV.
    void divApuTick();

    /// Hardware quirk: powering the APU on while the DIV-APU bit is high
    /// swallows the next DIV-APU event.
    void skipNextDivApuTick() { skipDivApuEvent = true; }

    /// Current un-filtered mixer output, in units of 1/8 of a channel step.
    /// Exposed for tests and diagnostics.
    int32_t mixedLeft() const { return mixLeft; }
    int32_t mixedRight() const { return mixRight; }

    uint8_t frameSequencerStep() const { return frameStep; }
    bool isPowered() const { return powered; }

private:
    struct Biquad
    {
        double b0 = 1;
        double b1 = 0;
        double b2 = 0;
        double a1 = 0;
        double a2 = 0;
        double x1 = 0;
        double x2 = 0;
        double y1 = 0;
        double y2 = 0;

        void configure(double cutoff, double sampleRate);
        double process(double input);
        void reset();
    };

    void clockFrameSequencer();
    void mixAndAccumulate();
    void emitSample();
    void flushSamples();

    std::unique_ptr<SoundInterface> sound;
    std::array<std::unique_ptr<GbChannelInterface>, 4> channels;
    bool cgb = true;

    uint8_t nr50 = 0x77;  // FF24 master volume
    uint8_t nr51 = 0xF3;  // FF25 panning
    bool powered = true;

    uint8_t frameStep = 0;
    bool skipDivApuEvent = false;

    uint32_t sampleAccumulator = 0;
    uint32_t sampleCycles = 0;
    int64_t mixAccumLeft = 0;
    int64_t mixAccumRight = 0;
    int32_t mixLeft = 0;
    int32_t mixRight = 0;
    bool dacsEnabled = false;

    // One-pole DC blocker, coefficient derived from the documentation's
    // 0.999958^(4194304/44100) formula.
    static constexpr double HighPassCharge = 0.996;
    double capacitorLeft = 0;
    double capacitorRight = 0;

    // Gentle output low-pass (about 10 kHz) to take the edge off the square
    // waves, similar to the analogue output stage of the handheld.
    static constexpr double LowPassCutoff = 10000.0;
    Biquad lowPassLeft;
    Biquad lowPassRight;

    uint32_t fadeInFrames = 0;
    uint32_t fadeInRemaining = 0;
    bool hadDacs = false;

    static constexpr std::size_t SampleBatchFrames = 512;
    std::array<int16_t, SampleBatchFrames * 2> sampleBuffer{};
    std::size_t sampleCount = 0;
};

}  // namespace qnemu
