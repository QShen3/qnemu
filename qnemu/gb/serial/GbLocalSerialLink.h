/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

#include "qnemu/gb/serial/GbSerialLink.h"

namespace qnemu
{

/// Creates the two endpoints of an in-process link cable.
GbSerialLinkPair createLocalSerialLinkPair();

/**
 * In-process link endpoint.
 *
 * Both endpoints share one small state block.  The block is protected by a
 * mutex because the two Game Boys normally run on separate emulation threads;
 * the pending flag is atomic so an idle serial device can poll it every
 * T-cycle without touching the mutex.
 */
class GbLocalSerialLink final : public GbSerialLink
{
public:
    struct SharedState
    {
        std::mutex mutex;
        uint8_t shiftRegisters[2] {};
        uint8_t receivedBytes[2] {};
        std::atomic<bool> receivedPending[2] {};
        int clockOwner = -1;
    };

    GbLocalSerialLink(std::shared_ptr<SharedState> state, int index);

    void setShiftRegister(uint8_t value) override;
    bool beginTransfer(uint8_t& incoming) override;
    void finishTransfer(uint8_t outgoing) override;
    void abortTransfer() override;
    bool hasPendingByte() const override;
    bool takeReceivedByte(uint8_t& value) override;
    void reset() override;

private:
    int peerIndex() const { return index ^ 1; }

    std::shared_ptr<SharedState> state;
    int index;
};

}  // namespace qnemu
