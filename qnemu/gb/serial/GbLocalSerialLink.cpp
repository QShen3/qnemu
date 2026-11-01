/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/serial/GbLocalSerialLink.h"

#include <memory>
#include <mutex>
#include <utility>

namespace qnemu
{

GbSerialLinkPair createLocalSerialLinkPair()
{
    const auto state = std::make_shared<GbLocalSerialLink::SharedState>();
    return {
        std::make_shared<GbLocalSerialLink>(state, 0),
        std::make_shared<GbLocalSerialLink>(state, 1),
    };
}

GbLocalSerialLink::GbLocalSerialLink(std::shared_ptr<SharedState> sharedState, int endpointIndex) :
    state(std::move(sharedState)),
    index(endpointIndex)
{
}

void GbLocalSerialLink::setShiftRegister(uint8_t value)
{
    const std::lock_guard lock(state->mutex);
    state->shiftRegisters[index] = value;
}

bool GbLocalSerialLink::beginTransfer(uint8_t& incoming)
{
    const std::lock_guard lock(state->mutex);

    // The first internally clocked endpoint owns the byte.  If both games
    // request a transfer at the same time, the other one behaves like a slave
    // for this byte instead of shifting a second time.
    if (state->clockOwner == -1) {
        state->clockOwner = index;
    }
    incoming = state->shiftRegisters[peerIndex()];
    return state->clockOwner == index;
}

void GbLocalSerialLink::finishTransfer(uint8_t outgoing)
{
    const std::lock_guard lock(state->mutex);
    if (state->clockOwner != index) {
        return;
    }

    state->shiftRegisters[peerIndex()] = outgoing;
    state->receivedBytes[peerIndex()] = outgoing;
    state->receivedPending[peerIndex()].store(true);
    state->clockOwner = -1;
}

void GbLocalSerialLink::abortTransfer()
{
    const std::lock_guard lock(state->mutex);
    if (state->clockOwner == index) {
        state->clockOwner = -1;
    }
}

bool GbLocalSerialLink::hasPendingByte() const
{
    return state->receivedPending[index].load();
}

bool GbLocalSerialLink::takeReceivedByte(uint8_t& value)
{
    const std::lock_guard lock(state->mutex);
    if (!state->receivedPending[index].load()) {
        return false;
    }

    state->receivedPending[index].store(false);
    value = state->receivedBytes[index];
    return true;
}

void GbLocalSerialLink::reset()
{
    const std::lock_guard lock(state->mutex);
    state->shiftRegisters[index] = 0;
    state->receivedBytes[index] = 0;
    state->receivedPending[index].store(false);
    if (state->clockOwner == index) {
        state->clockOwner = -1;
    }
}

}  // namespace qnemu
