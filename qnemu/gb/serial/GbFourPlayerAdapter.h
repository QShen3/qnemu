/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "qnemu/gb/serial/GbSerialLink.h"

namespace qnemu
{

/**
 * One endpoint of a DMG-07 four-player adapter.
 *
 * The adapter owns the serial clock for every Game Boy.  It implements the
 * ping/transmission protocol from Pan Docs, buffers one packet so data sent by
 * the players is broadcast in the next packet, and reports the connected
 * player mask through the ping STAT bytes.
 *
 * Endpoint 0 (Player 1) is the timebase: its GbSerial::step() advances the
 * adapter's packet schedule.  The other endpoints only consume bytes.
 */
class GbFourPlayerAdapter final : public GbSerialLink
{
public:
    struct SharedState;

    GbFourPlayerAdapter(std::shared_ptr<SharedState> state, int index);
    ~GbFourPlayerAdapter() override = default;

    void setShiftRegister(uint8_t value) override;
    bool beginTransfer(uint8_t& incoming) override;
    void finishTransfer(uint8_t /*outgoing*/) override {}
    void abortTransfer() override {}
    bool hasPendingByte() const override;
    bool takeReceivedByte(uint8_t& value) override;
    void reset() override;
    void setArmed(bool armed) override;
    void step() override;

private:
    void advanceLocked();
    void captureReplyLocked(uint8_t reply);

    std::shared_ptr<SharedState> state;
    int index;
};

/// Creates a four-player adapter with `playerCount` (2..4) attached ports.
std::vector<std::shared_ptr<GbSerialLink>> createFourPlayerAdapter(int playerCount);

}  // namespace qnemu
