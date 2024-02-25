/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

#include "QtCore/QBuffer"
#include "QtCore/QByteArray"
#include "QtMultimedia/QAudio"
#include "QtMultimedia/QAudioFormat"
#include "QtMultimedia/QAudioSink"

#include "qnemu/sound/SoundInterface.h"

namespace qnemu
{

class DefaultSound : public SoundInterface
{

public:
    DefaultSound() = delete;
    DefaultSound(int64_t sampleRate, const char* channelConfig, int bitDepth);
    ~DefaultSound() = default;

    void enable() override;
    void disable() override;

    void addData(char* data) override;

private:
    void handleAudioSinkStateChanged(QAudio::State newState);

    const int bufferSize = 4096;
    std::condition_variable cv;
    std::mutex mutex;
    int currentBufferSize = 0;
    bool isEnabled;
    QAudioFormat audioFormat;
    QBuffer buffer;
    QByteArray data;
    std::unique_ptr<QAudioSink> audioSink;
};

}  // namespace qnemu