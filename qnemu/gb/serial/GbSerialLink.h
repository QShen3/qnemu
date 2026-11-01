/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <utility>

namespace qnemu
{

/**
 * One endpoint of a Game Boy serial link.
 *
 * The interface models the byte exchange rather than the wire.  The endpoint
 * that selected the internal clock owns the transfer and calls
 * beginTransfer()/finishTransfer(); the other endpoint only observes the
 * completed byte.  This keeps the serial device independent of how the link
 * is implemented, so a local pair, a future socket transport or a printer
 * emulation can share the same code.
 */
class GbSerialLink
{
public:
    virtual ~GbSerialLink() = default;

    /// Publishes this endpoint's SB value to the peer.
    virtual void setShiftRegister(uint8_t value) = 0;

    /**
     * Starts an internally clocked byte exchange.
     *
     * @param incoming Receives the byte currently held by the peer's SB.
     * @return true if this endpoint owns the clock for the byte.  false means
     *         the peer is already clocking the byte; the caller must wait for
     *         takeReceivedByte().
     */
    virtual bool beginTransfer(uint8_t& incoming) = 0;

    /// Completes an internally clocked transfer and delivers `outgoing`.
    virtual void finishTransfer(uint8_t outgoing) = 0;

    /// Cancels a transfer owned by this endpoint.
    virtual void abortTransfer() = 0;

    /// Cheap per-cycle check, without taking the link lock.
    virtual bool hasPendingByte() const = 0;

    /// Consumes a byte clocked in by the peer.
    virtual bool takeReceivedByte(uint8_t& value) = 0;

    /// Clears all per-endpoint wire state (host reset / cartridge change).
    virtual void reset() = 0;
};

using GbSerialLinkPair =
    std::pair<std::shared_ptr<GbSerialLink>, std::shared_ptr<GbSerialLink>>;

}  // namespace qnemu
