/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/serial/GbSerial.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

namespace qnemu
{

namespace
{
constexpr uint16_t normalBitPeriod = 512;
constexpr uint16_t fastBitPeriod = 16;
}  // namespace

GbSerial::GbSerial(std::shared_ptr<GbInterruptHandlerInterface> interruptHandlerInterface,
        std::shared_ptr<GbSerialLink> serialLink, bool cgbMode) :
    interruptHandler(std::move(interruptHandlerInterface)),
    link(std::move(serialLink)),
    cgb(cgbMode)
{
    reset();
}

GbSerial::~GbSerial()
{
    if (link) {
        link->reset();
    }
}

uint8_t GbSerial::read(uint16_t address) const
{
    if (address == 0xFF01) {
        return shiftRegister;
    }

    assert(address == 0xFF02);
    // Bits 6-2 are unused and read as 1.  Bit 1 is only writable on CGB.
    return static_cast<uint8_t>((cgb ? 0x7C : 0x7E)
        | (control & (cgb ? 0x83 : 0x81)));
}

void GbSerial::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF01) {
        shiftRegister = value;
        if (link) {
            link->setShiftRegister(value);
        }
        return;
    }

    assert(address == 0xFF02);
    const uint8_t newControl = static_cast<uint8_t>(value & (cgb ? 0x83 : 0x81));

    if (transferActive) {
        // Clearing the start bit cancels the transfer.  Changing the clock
        // source restarts it; a running transfer keeps its current phase when
        // only the fast-clock bit changes.
        if ((newControl & 0x80) == 0
            || (newControl & 0x01) != (control & 0x01)) {
            abortTransfer();
        } else {
            control = newControl;
            return;
        }
    }

    control = newControl;
    if ((control & 0x80) != 0) {
        startTransfer();
    }
}

void GbSerial::step()
{
    if (link && link->hasPendingByte()) {
        uint8_t received = 0;
        if (link->takeReceivedByte(received)) {
            shiftRegister = received;
            control &= 0x7F;
            transferActive = false;
            clockOwner = false;
            interruptHandler->requestSerialInterrupt();
        }
    }

    if (!transferActive || !clockOwner) {
        return;
    }

    if (--ticksRemaining > 0) {
        return;
    }

    shiftRegister = static_cast<uint8_t>(
        (shiftRegister << 1) | ((incoming >> 7) & 0x01));
    incoming <<= 1;
    --bitsRemaining;

    if (bitsRemaining == 0) {
        control &= 0x7F;
        transferActive = false;
        clockOwner = false;
        if (link) {
            link->finishTransfer(outgoing);
        }
        interruptHandler->requestSerialInterrupt();
    } else {
        ticksRemaining = bitPeriod();
    }
}

void GbSerial::reset()
{
    shiftRegister = 0;
    // Post-boot SC is 0x7E on DMG and 0x7F on CGB.
    control = static_cast<uint8_t>(cgb ? 0x03 : 0x00);
    outgoing = 0;
    incoming = 0xFF;
    ticksRemaining = 0;
    bitsRemaining = 0;
    transferActive = false;
    clockOwner = false;

    if (link) {
        link->setShiftRegister(shiftRegister);
        link->reset();
    }
}

void GbSerial::setCgbMode(bool cgbMode)
{
    cgb = cgbMode;
    if (!cgb) {
        control &= 0x81;
    }
}

uint16_t GbSerial::bitPeriod() const
{
    uint16_t period = (cgb && (control & 0x02) != 0)
        ? fastBitPeriod
        : normalBitPeriod;
    if (doubleSpeed) {
        period = static_cast<uint16_t>(period / 2);
    }
    return period;
}

void GbSerial::startTransfer()
{
    transferActive = true;
    clockOwner = false;
    outgoing = shiftRegister;
    incoming = 0xFF;

    // External clock: the transfer only completes when the peer clocks it.
    if ((control & 0x01) == 0) {
        return;
    }

    if (link) {
        clockOwner = link->beginTransfer(incoming);
    } else {
        // No cable: the input line is pulled high, so SB receives 0xFF.
        incoming = 0xFF;
        clockOwner = true;
    }

    if (clockOwner) {
        bitsRemaining = 8;
        ticksRemaining = bitPeriod();
    }
}

void GbSerial::abortTransfer()
{
    transferActive = false;
    clockOwner = false;
    ticksRemaining = 0;
    bitsRemaining = 0;
    if (link) {
        link->abortTransfer();
    }
}

}  // namespace qnemu
