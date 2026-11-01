/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"
#include "qnemu/gb/serial/GbSerialLink.h"

namespace qnemu
{

/**
 * Serial data transfer unit (FF01 SB / FF02 SC).
 *
 * A transfer takes eight bit periods.  The normal clock is 8192 Hz, i.e. 512
 * T-cycles per bit; CGB fast mode is 262144 Hz (16 T-cycles per bit).  Both
 * rates double in CGB double-speed mode, which halves the number of
 * peripheral cycles per bit because the CPU bus already advances peripherals
 * at half rate.
 *
 * The link only carries whole bytes.  The clock owner still shifts its own
 * SB on every bit period so software that samples it mid-transfer sees the
 * hardware blend; the peer's byte is captured when the transfer starts and is
 * delivered when it ends.
 */
class GbSerial
{
public:
    GbSerial(std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::shared_ptr<GbSerialLink> link, bool cgb);
    ~GbSerial();

    GbSerial(const GbSerial&) = delete;
    GbSerial& operator=(const GbSerial&) = delete;

    uint8_t read(uint16_t address) const;
    void write(uint16_t address, const uint8_t& value);
    void step();
    void reset();

    /// CGB mode enables the fast clock and makes SC bit 1 writable.
    void setCgbMode(bool cgb);

    /// The CGB serial clock doubles in double-speed mode.
    void setDoubleSpeed(bool enabled) { doubleSpeed = enabled; }

    bool isTransferActive() const { return transferActive; }

private:
    uint16_t bitPeriod() const;
    void startTransfer();
    void abortTransfer();

    std::shared_ptr<GbInterruptHandlerInterface> interruptHandler;
    std::shared_ptr<GbSerialLink> link;

    uint8_t shiftRegister = 0;
    uint8_t control = 0;
    uint8_t outgoing = 0;
    uint8_t incoming = 0xFF;
    uint16_t ticksRemaining = 0;
    uint8_t bitsRemaining = 0;
    bool transferActive = false;
    bool clockOwner = false;
    bool cgb = false;
    bool doubleSpeed = false;
};

}  // namespace qnemu
