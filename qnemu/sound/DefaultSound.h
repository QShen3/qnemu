/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include <QtCore/QIODevice>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioSink>

#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

/**
 * Qt Multimedia audio output.
 *
 * A small thread-safe ring buffer sits between the emulation thread and
 * QAudioSink.  The sink pulls frames in real time; if the ring stays full, new
 * samples are dropped after a short wait so a stalled audio device cannot
 * freeze emulation.  Underruns are filled with silence instead of stopping the
 * sink.
 */
class DefaultSound : public SoundInterface
{
public:
    explicit DefaultSound(int sampleRate = 44100, int channels = 2);
    ~DefaultSound() override;

    DefaultSound(const DefaultSound&) = delete;
    DefaultSound& operator=(const DefaultSound&) = delete;

    void enable() override;
    void disable() override;
    void flush() override;
    void addSamples(const int16_t* samples, std::size_t frameCount) override;

private:
    class Stream;

    static constexpr std::size_t BufferFrames = 2048;

    QAudioFormat audioFormat;
    std::unique_ptr<Stream> stream;
    std::unique_ptr<QAudioSink> audioSink;
    std::atomic_bool enabled{true};
    std::atomic_bool sinkActive{false};
};

}  // namespace qnemu
