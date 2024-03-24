/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <cstring>
#include <string>

#include "QtCore/QObject"
#include "QtCore/QIODevice"
#include "QtMultimedia/QAudio"
#include "QtMultimedia/QAudioFormat"
#include "QtMultimedia/QAudioSink"

#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

DefaultSound::DefaultSound(int64_t sampleRate, const char* channelConfig, int bitDepth)
{
    audioFormat.setSampleRate(sampleRate);

    if (strcmp("1.0", channelConfig) == 0) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigMono);
    } else if (strcmp("2.0", channelConfig)) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigStereo);
    } else if (strcmp("2.1", channelConfig)) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfig2Dot1);
    } else if (strcmp("5.0", channelConfig)) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigSurround5Dot0);
    } else if (strcmp("5.1", channelConfig)) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigSurround5Dot1);
    } else if (strcmp("7.0", channelConfig)) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigSurround7Dot0);
    } else if (strcmp("7.1", channelConfig)) {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigSurround7Dot1);
    } else {
        audioFormat.setChannelConfig(QAudioFormat::ChannelConfigUnknown);
    }

    if (bitDepth == 8) {
        audioFormat.setSampleFormat(QAudioFormat::UInt8);
    } else if (bitDepth == 16) {
        audioFormat.setSampleFormat(QAudioFormat::Int16);
    } else if (bitDepth == 32) {
        audioFormat.setSampleFormat(QAudioFormat::Float);
    } else {
        audioFormat.setSampleFormat(QAudioFormat::Unknown);
    }

    data.resize(audioFormat.bytesPerSample() * bufferSize);
    buffer.setBuffer(&data);
    buffer.open(QIODevice::ReadWrite);

    audioSink = std::make_unique<QAudioSink>(audioFormat);

    QObject::connect(audioSink.get(), &QAudioSink::stateChanged, [this](QAudio::State newState){ handleAudioSinkStateChanged(newState); });
}

void DefaultSound::addData(char* data)
{
    std::unique_lock lock(mutex);
    buffer.write(data, audioFormat.bytesPerSample());
    currentBufferSize++;
    if (currentBufferSize >= bufferSize) {
        buffer.reset();
        audioSink->start(&buffer);
        cv.wait(lock, [this](){ return currentBufferSize == 0; });
    }
}

void DefaultSound::enable()
{
    isEnabled = true;
}

void DefaultSound::disable()
{
    isEnabled = false;
}

void DefaultSound::handleAudioSinkStateChanged(QAudio::State newState)
{
    if (newState == QAudio::IdleState) {
        const std::lock_guard<std::mutex> lock(mutex);
        buffer.reset();
        currentBufferSize = 0;
        cv.notify_one();
    }
}

}  // namespace qnemu