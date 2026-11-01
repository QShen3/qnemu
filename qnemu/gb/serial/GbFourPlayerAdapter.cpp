/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/gb/serial/GbFourPlayerAdapter.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace qnemu
{

namespace
{

// The adapter's schedule is advanced by Player 1's peripheral clock.  These
// constants are Pan Docs timings rounded to 4.194304 MHz T-cycles.
constexpr uint64_t cyclesPerMillisecond = 4194;
constexpr uint64_t pingInterByteCycles = 5955;       // 1.42 ms
constexpr uint64_t pingPacketGapBase = 51543;        // 12.29 ms
constexpr uint64_t transferInterByteBase = 3720;     // 0.887 ms
constexpr uint64_t transferInterByteStep = 445;      // 0.106 ms
constexpr uint64_t transferPacketGapBase = 71303;    // 17 ms
constexpr uint64_t transferByteTime = 537;           // 0.128 ms
constexpr uint64_t readyRetryCycles = 128;
constexpr int maxPacketBytes = 16;

uint64_t pingPacketGap(uint8_t rate)
{
    return pingPacketGapBase + (rate & 0x0F) * cyclesPerMillisecond;
}

uint64_t transferInterByte(uint8_t rate)
{
    return transferInterByteBase + (rate >> 4) * transferInterByteStep;
}

uint64_t transferPacketGap(uint8_t rate, int packetBytes)
{
    const uint64_t minimum = transferPacketGapBase
        + (rate & 0x0F) * cyclesPerMillisecond;
    const uint64_t byteTime = (transferByteTime + transferInterByte(rate))
            * static_cast<uint64_t>(packetBytes)
        + cyclesPerMillisecond;
    return std::max(minimum, byteTime);
}

}  // namespace

struct GbFourPlayerAdapter::SharedState
{
    struct Port
    {
        uint8_t shiftRegister = 0;
        uint8_t pendingByte = 0;
        bool present = false;
        bool armed = false;
        bool awaitingCompletion = false;
        bool awaitingReplyCapture = false;
        bool everArmed = false;
        bool connected = false;
        int captureByteIndex = 0;
        uint8_t pingReply[4] {};
        int aaRun = 0;
        int ffRun = 0;
    };

    enum class Phase
    {
        ping,
        ccIndicator,
        transfer,
        restartIndicator,
    };

    std::mutex mutex;
    Port ports[4];
    std::atomic<bool> pending[4] {};
    std::atomic<uint64_t> cyclesUntilByte {0};
    int portCount = 0;

    Phase phase = Phase::ping;
    int byteIndex = 0;
    int packetBytes = 4;
    uint8_t rate = 0x10;
    int size = 1;
    uint8_t connectedMask = 0;
    std::array<uint8_t, maxPacketBytes> previousPacket {};
    std::array<uint8_t, maxPacketBytes> currentPacket {};
};

GbFourPlayerAdapter::GbFourPlayerAdapter(std::shared_ptr<SharedState> sharedState,
    int endpointIndex) :
    state(std::move(sharedState)),
    index(endpointIndex)
{
}

void GbFourPlayerAdapter::setShiftRegister(uint8_t value)
{
    const std::lock_guard lock(state->mutex);
    SharedState::Port& port = state->ports[index];
    port.shiftRegister = value;
    if (port.awaitingReplyCapture) {
        captureReplyLocked(value);
    }
}

void GbFourPlayerAdapter::captureReplyLocked(uint8_t reply)
{
    SharedState::Port& port = state->ports[index];
    if (!port.awaitingReplyCapture) {
        return;
    }

    const int replyIndex = port.captureByteIndex;
    if (state->phase == SharedState::Phase::ping) {
        if (replyIndex >= 0 && replyIndex < 4) {
            port.pingReply[replyIndex] = reply;
        }
        if (reply == 0xAA) {
            ++port.aaRun;
        } else {
            port.aaRun = 0;
        }
    } else if (state->phase == SharedState::Phase::transfer) {
        if (replyIndex < state->size) {
            state->currentPacket[index * state->size + replyIndex] = reply;
        }
        if (reply == 0xFF) {
            ++port.ffRun;
        } else {
            port.ffRun = 0;
        }
    }
    port.awaitingReplyCapture = false;
}

bool GbFourPlayerAdapter::beginTransfer(uint8_t& incoming)
{
    // The DMG-07 owns the clock.  If software selects the internal clock,
    // treat it as if it were externally clocked instead of double-clocking.
    incoming = 0xFF;
    return false;
}

bool GbFourPlayerAdapter::hasPendingByte() const
{
    return state->pending[index].load(std::memory_order_acquire);
}

bool GbFourPlayerAdapter::takeReceivedByte(uint8_t& value)
{
    const std::lock_guard lock(state->mutex);
    if (!state->pending[index].exchange(false, std::memory_order_acq_rel)) {
        return false;
    }
    value = state->ports[index].pendingByte;
    state->ports[index].armed = false;
    state->ports[index].awaitingCompletion = false;
    return true;
}

void GbFourPlayerAdapter::setArmed(bool armed)
{
    const std::lock_guard lock(state->mutex);
    SharedState::Port& port = state->ports[index];
    if (armed) {
        // The ISR normally writes SB first and then re-arms.  If it only
        // re-arms, use the value currently in the shift register as its reply.
        if (port.awaitingReplyCapture) {
            captureReplyLocked(port.shiftRegister);
        }
        port.awaitingCompletion = false;
        port.everArmed = true;
    }
    port.armed = armed;
}

void GbFourPlayerAdapter::reset()
{
    const std::lock_guard lock(state->mutex);
    SharedState::Port& port = state->ports[index];
    port.shiftRegister = 0;
    port.pendingByte = 0;
    port.armed = false;
    port.awaitingCompletion = false;
    port.awaitingReplyCapture = false;
    port.everArmed = false;
    port.connected = false;
    port.captureByteIndex = 0;
    port.aaRun = 0;
    port.ffRun = 0;
    port.pingReply[0] = 0;
    port.pingReply[1] = 0;
    port.pingReply[2] = 0;
    port.pingReply[3] = 0;
    state->pending[index].store(false, std::memory_order_release);

    if (index != 0) {
        return;
    }

    state->phase = SharedState::Phase::ping;
    state->byteIndex = 0;
    state->packetBytes = 4;
    state->rate = 0x10;
    state->size = 1;
    state->connectedMask = 0;
    state->previousPacket.fill(0);
    state->currentPacket.fill(0);
    state->cyclesUntilByte.store(0, std::memory_order_relaxed);
    for (SharedState::Port& other : state->ports) {
        other.armed = false;
        other.awaitingCompletion = false;
        other.awaitingReplyCapture = false;
        other.everArmed = false;
        other.connected = false;
        other.aaRun = 0;
        other.ffRun = 0;
    }
    for (int pendingPort = 0; pendingPort < 4; ++pendingPort) {
        state->pending[pendingPort].store(false, std::memory_order_release);
    }
}

void GbFourPlayerAdapter::step()
{
    if (index != 0) {
        return;
    }

    // Fast path: only Player 1's thread touches the countdown.
    if (state->cyclesUntilByte.load(std::memory_order_relaxed) > 0) {
        state->cyclesUntilByte.fetch_sub(1, std::memory_order_relaxed);
        return;
    }

    const std::lock_guard lock(state->mutex);
    advanceLocked();
}

void GbFourPlayerAdapter::advanceLocked()
{
    // The last byte of the packet has been clocked.  Wait until every game has
    // written its reply before completing the packet.
    if (state->byteIndex >= state->packetBytes) {
        for (int port = 0; port < state->portCount; ++port) {
            if (state->ports[port].awaitingReplyCapture) {
                state->cyclesUntilByte.store(readyRetryCycles,
                    std::memory_order_relaxed);
                return;
            }
        }

        const SharedState::Phase completedPhase = state->phase;
        state->byteIndex = 0;
        switch (completedPhase) {
        case SharedState::Phase::ping: {
            uint8_t mask = 0;
            for (int port = 0; port < state->portCount; ++port) {
                SharedState::Port& entry = state->ports[port];
                entry.connected = entry.present && entry.everArmed;
                if (entry.connected) {
                    mask = static_cast<uint8_t>(mask | (1U << port));
                }
            }
            state->connectedMask = mask;

            bool switchToTransfer = false;
            for (int port = 0; port < state->portCount; ++port) {
                if (state->ports[port].present
                    && state->ports[port].aaRun >= 4) {
                    switchToTransfer = true;
                }
                state->ports[port].aaRun = 0;
            }

            // The AA packet that starts the mode switch carries AA in the
            // RATE/SIZE slots; keep the last real ping settings instead.
            if (!switchToTransfer) {
                const uint8_t playerOneRate = state->ports[0].pingReply[2];
                if (playerOneRate != 0) {
                    state->rate = playerOneRate;
                }
                const uint8_t playerOneSize = state->ports[0].pingReply[3];
                if (playerOneSize >= 1 && playerOneSize <= 4) {
                    state->size = playerOneSize;
                }
            }

            state->phase = switchToTransfer
                ? SharedState::Phase::ccIndicator
                : SharedState::Phase::ping;
            state->packetBytes = 4;
            state->cyclesUntilByte.store(pingPacketGap(state->rate),
                std::memory_order_relaxed);
            return;
        }
        case SharedState::Phase::ccIndicator:
            state->phase = SharedState::Phase::transfer;
            state->packetBytes = state->size * 4;
            state->previousPacket.fill(0);
            state->currentPacket.fill(0);
            state->cyclesUntilByte.store(
                transferPacketGap(state->rate, state->packetBytes),
                std::memory_order_relaxed);
            return;
        case SharedState::Phase::transfer: {
            state->previousPacket = state->currentPacket;
            state->currentPacket.fill(0);

            bool restart = false;
            for (int port = 0; port < state->portCount; ++port) {
                if (state->ports[port].present
                    && state->ports[port].ffRun >= 3) {
                    restart = true;
                }
                state->ports[port].ffRun = 0;
            }
            state->phase = restart
                ? SharedState::Phase::restartIndicator
                : SharedState::Phase::transfer;
            state->packetBytes = state->size * 4;
            state->cyclesUntilByte.store(
                transferPacketGap(state->rate, state->packetBytes),
                std::memory_order_relaxed);
            return;
        }
        case SharedState::Phase::restartIndicator:
            state->phase = SharedState::Phase::ping;
            state->packetBytes = 4;
            state->connectedMask = 0;
            for (int port = 0; port < state->portCount; ++port) {
                state->ports[port].connected = false;
                state->ports[port].aaRun = 0;
                state->ports[port].ffRun = 0;
                state->ports[port].everArmed = false;
            }
            state->cyclesUntilByte.store(pingPacketGap(state->rate),
                std::memory_order_relaxed);
            return;
        }
    }

    // Wait for every attached Game Boy to arm for the next byte.
    for (int port = 0; port < state->portCount; ++port) {
        if (!state->ports[port].present) {
            continue;
        }
        if (!state->ports[port].armed
            || state->ports[port].awaitingCompletion
            || state->ports[port].awaitingReplyCapture) {
            state->cyclesUntilByte.store(readyRetryCycles,
                std::memory_order_relaxed);
            return;
        }
    }

    const uint8_t connectedMask = state->connectedMask;
    for (int portIndex = 0; portIndex < state->portCount; ++portIndex) {
        SharedState::Port& port = state->ports[portIndex];
        if (!port.present) {
            continue;
        }

        uint8_t received = 0;
        switch (state->phase) {
        case SharedState::Phase::ping:
            received = state->byteIndex == 0
                ? 0xFE
                : static_cast<uint8_t>((connectedMask << 4) | (portIndex + 1));
            break;
        case SharedState::Phase::ccIndicator:
            received = 0xCC;
            break;
        case SharedState::Phase::transfer: {
            const int player = state->byteIndex / state->size;
            const int offset = state->byteIndex % state->size;
            if (player < state->portCount) {
                received = state->previousPacket[player * state->size + offset];
            }
            break;
        }
        case SharedState::Phase::restartIndicator:
            received = 0xFF;
            break;
        }

        port.pendingByte = received;
        port.awaitingCompletion = true;
        port.awaitingReplyCapture = true;
        port.captureByteIndex = state->byteIndex;
        port.armed = false;
        state->pending[portIndex].store(true, std::memory_order_release);
    }

    ++state->byteIndex;
    if (state->byteIndex < state->packetBytes) {
        state->cyclesUntilByte.store(
            state->phase == SharedState::Phase::ping
                    || state->phase == SharedState::Phase::ccIndicator
                ? pingInterByteCycles
                : transferInterByte(state->rate),
            std::memory_order_relaxed);
    } else {
        // The next advanceLocked() waits for the last replies and completes
        // the packet.
        state->cyclesUntilByte.store(readyRetryCycles,
            std::memory_order_relaxed);
    }
}

std::vector<std::shared_ptr<GbSerialLink>> createFourPlayerAdapter(int playerCount)
{
    const int count = std::clamp(playerCount, 2, 4);
    const auto state = std::make_shared<GbFourPlayerAdapter::SharedState>();
    state->portCount = count;
    for (int port = 0; port < count; ++port) {
        state->ports[port].present = true;
    }

    std::vector<std::shared_ptr<GbSerialLink>> endpoints;
    endpoints.reserve(count);
    for (int port = 0; port < count; ++port) {
        endpoints.push_back(
            std::make_shared<GbFourPlayerAdapter>(state, port));
    }
    return endpoints;
}

}  // namespace qnemu
