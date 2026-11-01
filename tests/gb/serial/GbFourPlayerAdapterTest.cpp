/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "qnemu/gb/serial/GbFourPlayerAdapter.h"

namespace qnemuTest
{

namespace
{

class FakePlayer
{
public:
    FakePlayer(std::shared_ptr<qnemu::GbSerialLink> endpoint, uint8_t transmit)
        : link(std::move(endpoint)), tx(transmit)
    {
    }

    void poll()
    {
        uint8_t byte = 0;
        while (link->hasPendingByte() && link->takeReceivedByte(byte)) {
            received.push_back(byte);
            process(byte);
        }
    }

    void arm() { link->setArmed(true); }
    bool sawPingAfterRestart() const { return seenPingAfterRestart; }

    std::shared_ptr<qnemu::GbSerialLink> link;
    uint8_t tx = 0;
    std::vector<uint8_t> received;

private:
    void process(uint8_t byte)
    {
        if (inTransfer) {
            if (byte == 0xFF) {
                if (++ffReceived >= 4) {
                    inTransfer = false;
                    requestXfer = false;
                    requestRestart = false;
                    pingState = 0;
                    pingPackets = 0;
                    ccCount = 0;
                    xferCount = 0;
                    ffReceived = 0;
                    xferPackets = 0;
                    seenPingAfterRestart = true;
                    replyByte(0);
                    return;
                }
            } else {
                ffReceived = 0;
            }

            if (xferCount == 0) {
                ++xferPackets;
                if (xferPackets >= 4) {
                    requestRestart = true;
                }
            }
            const uint8_t reply = requestRestart
                ? 0xFF
                : (xferCount == 0 ? tx : 0);
            xferCount = (xferCount + 1) % 4;
            replyByte(reply);
            return;
        }

        if (byte == 0xCC) {
            if (++ccCount >= 4) {
                inTransfer = true;
                xferCount = 0;
            }
            replyByte(0);
            return;
        }
        ccCount = 0;

        if (byte == 0xFE) {
            pingState = 1;
            ++pingPackets;
            requestXfer = pingPackets >= 3;
            if (requestXfer) {
                replyByte(0xAA);
            } else {
                replyByte(0x88);
            }
            return;
        }

        if (pingState == 1) {
            pingState = 2;
            replyByte(requestXfer ? 0xAA : 0x88);
        } else if (pingState == 2) {
            pingState = 3;
            replyByte(requestXfer ? 0xAA : 0x10);
        } else if (pingState == 3) {
            pingState = 4;
            replyByte(requestXfer ? 0xAA : 0x01);
        } else {
            replyByte(0);
        }
    }

    void replyByte(uint8_t value)
    {
        link->setShiftRegister(value);
        link->setArmed(true);
    }

    bool inTransfer = false;
    bool requestXfer = false;
    bool requestRestart = false;
    bool seenPingAfterRestart = false;
    int pingPackets = 0;
    int pingState = 0;
    int ccCount = 0;
    int xferCount = 0;
    int xferPackets = 0;
    int ffReceived = 0;
};

bool containsSequence(const std::vector<uint8_t>& bytes,
    const std::vector<uint8_t>& sequence)
{
    if (sequence.empty() || bytes.size() < sequence.size()) {
        return false;
    }
    for (std::size_t start = 0; start + sequence.size() <= bytes.size(); ++start) {
        bool match = true;
        for (std::size_t offset = 0; offset < sequence.size(); ++offset) {
            if (bytes[start + offset] != sequence[offset]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

}  // namespace

TEST(GbFourPlayerAdapterTest, PingsConnectsAndExchangesFourPlayerPackets)
{
    const auto links = qnemu::createFourPlayerAdapter(4);
    ASSERT_EQ(links.size(), 4U);

    std::array<std::unique_ptr<FakePlayer>, 4> players;
    for (int index = 0; index < 4; ++index) {
        players.at(index) = std::make_unique<FakePlayer>(
            links.at(index), static_cast<uint8_t>(0x10 + index));
        players.at(index)->arm();
    }

    for (int cycle = 0; cycle < 1500000; ++cycle) {
        for (auto& player : players) {
            player->poll();
        }
        links.at(0)->step();
    }
    for (auto& player : players) {
        player->poll();
    }

    for (int index = 0; index < 4; ++index) {
        const std::vector<uint8_t>& received = players.at(index)->received;
        EXPECT_TRUE(containsSequence(received, {0xFE}))
            << "player " << index << " never received a ping header";
        EXPECT_TRUE(containsSequence(received,
            {static_cast<uint8_t>(0xF0 | (index + 1))}))
            << "player " << index << " never saw the full connection mask";
        EXPECT_TRUE(containsSequence(received, {0xCC, 0xCC, 0xCC, 0xCC}))
            << "player " << index << " never received the transfer indicator";
        EXPECT_TRUE(containsSequence(received, {0x10, 0x11, 0x12, 0x13}))
            << "player " << index << " never received the first data packet";
        EXPECT_TRUE(players.at(index)->sawPingAfterRestart())
            << "player " << index << " did not return to ping mode";
    }
}

TEST(GbFourPlayerAdapterTest, ReportsOnlyAttachedPlayers)
{
    const auto links = qnemu::createFourPlayerAdapter(2);
    ASSERT_EQ(links.size(), 2U);

    std::array<std::unique_ptr<FakePlayer>, 2> players;
    for (int index = 0; index < 2; ++index) {
        players.at(index) = std::make_unique<FakePlayer>(
            links.at(index), static_cast<uint8_t>(0x20 + index));
        players.at(index)->arm();
    }

    for (int cycle = 0; cycle < 300000; ++cycle) {
        for (auto& player : players) {
            player->poll();
        }
        links.at(0)->step();
    }
    for (auto& player : players) {
        player->poll();
    }

    for (int index = 0; index < 2; ++index) {
        const std::vector<uint8_t>& received = players.at(index)->received;
        EXPECT_TRUE(containsSequence(received,
            {static_cast<uint8_t>(0x30 | (index + 1))}))
            << "player " << index << " never saw a two-player mask";
    }
}

}  // namespace qnemuTest
