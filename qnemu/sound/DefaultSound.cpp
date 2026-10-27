/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include <QtCore/QIODevice>
#include <QtMultimedia/QAudio>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>

#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

class DefaultSound::Stream : public QIODevice
{
public:
    Stream(int channels, std::size_t capacityFrames) :
        channels(channels),
        capacityFrames(capacityFrames),
        ring(static_cast<std::size_t>(channels) * capacityFrames, 0)
    {
    }

    ~Stream() override = default;

    void writeFrames(const int16_t* samples, std::size_t frameCount)
    {
        std::unique_lock lock(mutex);
        const int16_t* source = samples;
        std::size_t remaining = frameCount;

        while (remaining > 0 && !stopped) {
            const bool ready = spaceAvailable.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return stopped || sizeFrames < capacityFrames;
            });
            if (!ready || stopped) {
                return;
            }

            const std::size_t freeFrames = capacityFrames - sizeFrames;
            const std::size_t count = std::min(freeFrames, remaining);
            const std::size_t first = std::min(count, capacityFrames - writeFrame);

            std::memcpy(&ring[writeFrame * static_cast<std::size_t>(channels)], source,
                first * static_cast<std::size_t>(channels) * sizeof(int16_t));
            if (count > first) {
                std::memcpy(&ring[0],
                    source + first * static_cast<std::size_t>(channels),
                    (count - first) * static_cast<std::size_t>(channels) * sizeof(int16_t));
            }

            writeFrame = (writeFrame + count) % capacityFrames;
            sizeFrames += count;
            source += count * static_cast<std::size_t>(channels);
            remaining -= count;
        }
    }

    void clear()
    {
        const std::lock_guard lock(mutex);
        readFrame = 0;
        writeFrame = 0;
        sizeFrames = 0;
        spaceAvailable.notify_all();
    }

    void stop()
    {
        const std::lock_guard lock(mutex);
        stopped = true;
        spaceAvailable.notify_all();
    }

    qint64 bytesAvailable() const override
    {
        // Pull-mode QAudioSink stops asking for data when bytesAvailable()
        // reaches zero.  Report the whole ring as "available" and let
        // readData() return silence on underrun, so the sink keeps running.
        return static_cast<qint64>(capacityFrames * static_cast<std::size_t>(channels)
            * sizeof(int16_t));
    }

protected:
    qint64 readData(char* data, qint64 maxSize) override
    {
        if (maxSize <= 0) {
            return 0;
        }

        const std::size_t bytesPerFrame =
            static_cast<std::size_t>(channels) * sizeof(int16_t);
        const std::size_t requestedFrames = static_cast<std::size_t>(maxSize) / bytesPerFrame;
        const std::size_t remainder = static_cast<std::size_t>(maxSize) % bytesPerFrame;
        if (requestedFrames == 0) {
            std::memset(data, 0, static_cast<std::size_t>(maxSize));
            return maxSize;
        }

        std::size_t framesRead = 0;
        {
            const std::lock_guard lock(mutex);
            framesRead = std::min(requestedFrames, sizeFrames);
            auto* output = reinterpret_cast<int16_t*>(data);

            const std::size_t first = std::min(framesRead, capacityFrames - readFrame);
            std::memcpy(output, &ring[readFrame * static_cast<std::size_t>(channels)],
                first * bytesPerFrame);
            if (framesRead > first) {
                std::memcpy(output + first * static_cast<std::size_t>(channels),
                    &ring[0], (framesRead - first) * bytesPerFrame);
            }

            readFrame = (readFrame + framesRead) % capacityFrames;
            sizeFrames -= framesRead;

            if (framesRead < requestedFrames) {
                std::memset(output + framesRead * static_cast<std::size_t>(channels), 0,
                    (requestedFrames - framesRead) * bytesPerFrame);
            }
            if (remainder != 0) {
                std::memset(data + requestedFrames * bytesPerFrame, 0, remainder);
            }
        }

        spaceAvailable.notify_all();
        return maxSize;
    }

    qint64 writeData(const char*, qint64) override
    {
        return -1;
    }

    bool isSequential() const override { return true; }

private:
    int channels;
    std::size_t capacityFrames;
    std::vector<int16_t> ring;

    std::size_t readFrame = 0;
    std::size_t writeFrame = 0;
    std::size_t sizeFrames = 0;

    mutable std::mutex mutex;
    std::condition_variable spaceAvailable;
    bool stopped = false;
};

DefaultSound::DefaultSound(int sampleRate, int channels)
{
    audioFormat.setSampleRate(sampleRate);
    audioFormat.setChannelCount(channels);
    audioFormat.setSampleFormat(QAudioFormat::Int16);

    stream = std::make_unique<Stream>(channels, BufferFrames);
    stream->open(QIODevice::ReadOnly);

    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        return;
    }

    audioSink = std::make_unique<QAudioSink>(device, audioFormat);
    // Keep the device buffer small (about 23 ms at 44.1 kHz stereo); the ring
    // buffer above provides the larger cushion.
    audioSink->setBufferSize(
        static_cast<int>(BufferFrames / 2 * static_cast<std::size_t>(channels) * sizeof(int16_t)));

    QObject::connect(audioSink.get(), &QAudioSink::stateChanged, audioSink.get(),
        [this](QAudio::State state) {
            sinkActive = state != QAudio::StoppedState;
        });

    audioSink->start(stream.get());
    if (audioSink->state() == QAudio::StoppedState) {
        audioSink.reset();
    } else {
        sinkActive = true;
    }
}

DefaultSound::~DefaultSound()
{
    sinkActive = false;
    if (audioSink) {
        // The stateChanged lambda captures `this`; make sure no signal can
        // arrive while the members are being destroyed.
        QObject::disconnect(audioSink.get(), nullptr, nullptr, nullptr);
        audioSink->stop();
    }
    if (stream) {
        stream->stop();
    }
}

void DefaultSound::enable()
{
    enabled = true;
    if (audioSink && !sinkActive.load()) {
        audioSink->start(stream.get());
        sinkActive = audioSink->state() != QAudio::StoppedState;
    }
}

void DefaultSound::disable()
{
    enabled = false;
    if (stream) {
        stream->clear();
    }
}

void DefaultSound::flush()
{
    if (stream) {
        stream->clear();
    }
}

void DefaultSound::addSamples(const int16_t* samples, std::size_t frameCount)
{
    if (!enabled.load() || !sinkActive.load() || !stream || frameCount == 0) {
        return;
    }

    stream->writeFrames(samples, frameCount);
}

}  // namespace qnemu
