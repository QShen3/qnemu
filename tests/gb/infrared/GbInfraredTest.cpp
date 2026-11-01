/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/infrared/GbInfrared.h"
#include "qnemu/gb/infrared/GbLocalInfraredLink.h"

namespace qnemuTest
{

namespace
{

constexpr uint16_t rpAddress = 0xFF56;

struct InfraredPair
{
    std::unique_ptr<qnemu::GbInfrared> first;
    std::unique_ptr<qnemu::GbInfrared> second;
};

InfraredPair makePair()
{
    const auto [firstLink, secondLink] = qnemu::createLocalInfraredLinkPair();
    return {
        std::make_unique<qnemu::GbInfrared>(firstLink),
        std::make_unique<qnemu::GbInfrared>(secondLink),
    };
}

}  // namespace

TEST(GbInfraredTest, ResetValueAndUnusedBits)
{
    qnemu::GbInfrared infrared(nullptr);
    EXPECT_EQ(infrared.read(rpAddress), 0x3E);

    infrared.write(rpAddress, 0xFF);
    // Only bit 0 and bits 6-7 are writable; bits 2-5 read high and there is no
    // peer, so bit 1 is high.
    EXPECT_EQ(infrared.read(rpAddress), 0xFF);

    infrared.write(rpAddress, 0x3F);
    EXPECT_EQ(infrared.read(rpAddress), 0x3F);
}

TEST(GbInfraredTest, PeerLedRequiresReadEnable)
{
    auto pair = makePair();

    pair.first->write(rpAddress, 0x01);  // LED on
    pair.second->write(rpAddress, 0xC0); // read enable
    EXPECT_EQ(pair.second->read(rpAddress), 0xFC);

    pair.second->write(rpAddress, 0x00); // reading disabled
    EXPECT_EQ(pair.second->read(rpAddress), 0x3E);

    pair.second->write(rpAddress, 0xC0);
    pair.first->write(rpAddress, 0x00); // LED off
    EXPECT_EQ(pair.second->read(rpAddress), 0xFE);
}

TEST(GbInfraredTest, ContinuousSignalFadesAfterThreeMilliseconds)
{
    auto pair = makePair();
    pair.first->write(rpAddress, 0x01);
    pair.second->write(rpAddress, 0xC0);

    EXPECT_EQ(pair.second->read(rpAddress), 0xFC);

    for (int cycle = 0; cycle < 13000; ++cycle) {
        pair.second->step();
    }
    EXPECT_EQ(pair.second->read(rpAddress), 0xFE);

    pair.first->write(rpAddress, 0x00);
    pair.second->step();
    pair.first->write(rpAddress, 0x01);
    pair.second->step();
    EXPECT_EQ(pair.second->read(rpAddress), 0xFC);
}

TEST(GbInfraredTest, ResetDropsTheLed)
{
    auto pair = makePair();
    pair.first->write(rpAddress, 0xC1);
    pair.second->write(rpAddress, 0xC0);
    EXPECT_EQ(pair.second->read(rpAddress), 0xFC);

    pair.first->reset();
    EXPECT_EQ(pair.first->read(rpAddress), 0x3E);
    EXPECT_EQ(pair.second->read(rpAddress), 0xFE);
}

}  // namespace qnemuTest
