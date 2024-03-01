/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/apu/GbChannel2.h"

namespace qnemu
{

GbChannel2::GbChannel2()
{
    reset();
}

uint8_t GbChannel2::read(uint16_t address) const
{
    if (address == 0xFF16) {
        return registers.channel2LengthTimerAndDutyCycle;
    } else if (address == 0xFF17) {
        return registers.channel2VolumeAndEnvelope;
    } else if (address == 0xFF18) {
        return registers.channel2PeriodLow;
    } else if (address == 0xFF19) {
        return registers.channel2PeriodHighAndControl;
    }
    return 0xFF;
}

void GbChannel2::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF16) {
        registers.channel2LengthTimerAndDutyCycle = value;
        lengthTimer.setLength(64 - registers.channel2InitialLengthTimer);
    } else if (address == 0xFF17) {
        registers.channel2VolumeAndEnvelope = value;
        volumeEnvelope.setInitialVolume(registers.channel2InitialVolume);
        volumeEnvelope.setDirection(registers.channel2EnvelopeDirection == 1 ? true : false);
        volumeEnvelope.setPace(registers.channel2SweepPace);
        if (registers.channel2InitialVolume != 0 || registers.channel2EnvelopeDirection != 0) {
            enabled = true;
        } else {
            enabled = false;
        }
    } else if (address == 0xFF18) {
        registers.channel2PeriodLow = value;
    } else if (address == 0xFF19) {
        registers.channel2PeriodHighAndControl = value;
        if (registers.channel2LengthEnable) {
            lengthTimer.enable();
        } else {
            lengthTimer.disable();
        }
        if (registers.channel2Trigger) {
            enabled = true;
            counter = 1;
            currentBit = 0;
            lengthTimer.trigger();
            volumeEnvelope.trigger();
        }
    }
}

void GbChannel2::step()
{
    lengthTimer.step();
    volumeEnvelope.step();
    if (!enabled) {
        data = 0;
        return;
    }
    if (lengthTimer.isEnabled() && lengthTimer.getLength() == 0) {
        enabled = false;
        data = 0;
        return;
    }

    counter--;
    if (counter == 0) {
        counter = getFrequency();
        data = (getDuty() & (1 << currentBit)) >> currentBit;
        currentBit = (currentBit + 1) % 8;
    }

    data = data * volumeEnvelope.getVolume();
}

void GbChannel2::reset()
{
    registers.channel2LengthTimerAndDutyCycle = 0x3F;
    registers.channel2VolumeAndEnvelope = 0;
    registers.channel2PeriodLow = 0xFF;
    registers.channel2PeriodHighAndControl = 0xBF;

    data = 0;
    enabled = true;
    counter = 0;
    currentBit = 0;

    lengthTimer.reset();
    volumeEnvelope.reset();
}

uint8_t GbChannel2::getData() const
{
    return data;
}

uint16_t GbChannel2::getFrequency() const
{
    return (2048 - ((registers.channel2PeriodHigh << 8) | registers.channel2PeriodLow)) * 4;
}

uint8_t GbChannel2::getDuty() const
{
    if (registers.channel2WaveDuty == 0) {
        return 0b11111110;
    } else if (registers.channel2WaveDuty == 1) {
        return 0b01111110;
    } else if (registers.channel2WaveDuty == 2) {
        return 0b01111000;
    } else if (registers.channel2WaveDuty == 3) {
        return 0b10000001;
    }
    return 0;
}

}  // namespace qnemu