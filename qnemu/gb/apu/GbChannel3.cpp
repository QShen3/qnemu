/*
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel3.h"

namespace qnemu
{

namespace
{

constexpr uint16_t WaveRamStart = 0xFF30;
constexpr uint16_t WaveRamEnd = 0xFF3F;

bool isWaveRamAddress(uint16_t address)
{
    return address >= WaveRamStart && address <= WaveRamEnd;
}

}  // namespace

GbChannel3::GbChannel3()
{
    reset();
}

uint8_t GbChannel3::read(uint16_t address) const
{
    if (isWaveRamAddress(address)) {
        return readWaveRam(address);
    }

    switch (address) {
    case 0xFF1A:
        return static_cast<uint8_t>(0x7F | (nr30 & 0x80));
    case 0xFF1B:
        return 0xFF;
    case 0xFF1C:
        return static_cast<uint8_t>(0x9F | (nr32 & 0x60));
    case 0xFF1D:
        return 0xFF;
    case 0xFF1E:
        return static_cast<uint8_t>(0xBF | (nr34 & 0x40));
    default:
        return 0xFF;
    }
}

void GbChannel3::write(uint16_t address, const uint8_t& value)
{
    if (isWaveRamAddress(address)) {
        writeWaveRam(address, value);
        return;
    }

    switch (address) {
    case 0xFF1A:
        nr30 = static_cast<uint8_t>(value & 0x80);
        if (!isDacEnabled()) {
            enabled = false;
            data = 0;
        }
        break;
    case 0xFF1B:
        nr31 = value;
        lengthTimer.setLength(value);
        break;
    case 0xFF1C:
        nr32 = static_cast<uint8_t>(value & 0x60);
        break;
    case 0xFF1D:
        nr33 = value;
        break;
    case 0xFF1E:
        nr34 = static_cast<uint8_t>(value & 0xC7);
        lengthTimer.setEnabled((nr34 & 0x40) != 0);
        if ((nr34 & 0x80) != 0) {
            trigger();
        }
        break;
    default:
        break;
    }
}

void GbChannel3::trigger()
{
    if (!isDacEnabled()) {
        return;
    }

    enabled = true;
    lengthTimer.trigger();
    frequencyTimer = static_cast<uint16_t>((2048 - period()) * 2);

    // The hardware resets the index, then reads sample 1 first (the lower
    // nibble of FF30).  The previously played sample keeps being emitted
    // until the first wave RAM read happens.
    wavePosition = 0;
}

uint8_t GbChannel3::applyOutputLevel(uint8_t sample) const
{
    switch ((nr32 >> 5) & 0x03) {
    case 0:
        return 0;
    case 1:
        return sample;
    case 2:
        return static_cast<uint8_t>(sample >> 1);
    default:
        return static_cast<uint8_t>(sample >> 2);
    }
}

void GbChannel3::step()
{
    if (!enabled) {
        data = 0;
        return;
    }

    if (frequencyTimer > 0) {
        --frequencyTimer;
    }
    if (frequencyTimer == 0) {
        frequencyTimer = static_cast<uint16_t>((2048 - period()) * 2);
        wavePosition = static_cast<uint8_t>((wavePosition + 1) & 0x1F);
        const uint8_t byte = waveRam[wavePosition >> 1];
        sampleBuffer = (wavePosition & 0x01) != 0
            ? static_cast<uint8_t>(byte & 0x0F)
            : static_cast<uint8_t>((byte >> 4) & 0x0F);
    }

    data = applyOutputLevel(sampleBuffer);
}

void GbChannel3::clockLength()
{
    lengthTimer.clock();
    if (lengthTimer.isEnabled() && lengthTimer.expired()) {
        enabled = false;
        data = 0;
    }
}

uint8_t GbChannel3::readWaveRam(uint16_t address) const
{
    // While CH3 is active the CPU sees the byte the channel is currently
    // reading, regardless of the address it asked for (CGB behaviour).
    if (enabled && isDacEnabled()) {
        return waveRam[(wavePosition >> 1) & 0x0F];
    }
    return waveRam[(address - WaveRamStart) & 0x0F];
}

void GbChannel3::writeWaveRam(uint16_t address, uint8_t value)
{
    if (enabled && isDacEnabled()) {
        waveRam[(wavePosition >> 1) & 0x0F] = value;
        return;
    }
    waveRam[(address - WaveRamStart) & 0x0F] = value;
}

void GbChannel3::reset()
{
    nr30 = 0;
    nr31 = 0;
    nr32 = 0;
    nr33 = 0;
    nr34 = 0;

    enabled = false;
    frequencyTimer = 0;
    wavePosition = 0;
    sampleBuffer = 0;
    data = 0;

    lengthTimer.reset();
    waveRam.fill(0);
}

void GbChannel3::powerOff()
{
    // Wave RAM survives NR52 power-off.
    const auto savedWaveRam = waveRam;
    reset();
    waveRam = savedWaveRam;
}

}  // namespace qnemu
