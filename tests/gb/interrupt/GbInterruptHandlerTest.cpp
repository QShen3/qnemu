/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "qnemu/gb/interrupt/GbInterruptHandler.h"

namespace qnemuTest
{

TEST(GbInterruptTest, ResetValues)
{
    qnemu::GbInterruptHandler handler;

    EXPECT_EQ(handler.read(0xFF0F), 0xE1);
    EXPECT_EQ(handler.read(0xFFFF), 0x00);
    EXPECT_EQ(handler.pending(), 0x00);
}

TEST(GbInterruptTest, ReadAndWriteRegisters)
{
    qnemu::GbInterruptHandler handler;

    handler.write(0xFF0F, 0x1F);
    handler.write(0xFFFF, 0x1F);

    EXPECT_EQ(handler.read(0xFF0F), 0x1F);
    EXPECT_EQ(handler.read(0xFFFF), 0x1F);
}

TEST(GbInterruptTest, ResetClearsRequestsAndEnableMask)
{
    qnemu::GbInterruptHandler handler;
    handler.write(0xFF0F, 0xFF);
    handler.write(0xFFFF, 0xFF);

    handler.reset();

    EXPECT_EQ(handler.read(0xFF0F), 0xE1);
    EXPECT_EQ(handler.read(0xFFFF), 0x00);
}

TEST(GbInterruptTest, RequestSetsIndividualBits)
{
    qnemu::GbInterruptHandler handler;
    handler.write(0xFF0F, 0x00);

    handler.requestVBlankInterrupt();
    EXPECT_EQ(handler.read(0xFF0F), 0x01);

    handler.write(0xFF0F, 0x00);
    handler.requestLcdInterrupt();
    EXPECT_EQ(handler.read(0xFF0F), 0x02);

    handler.write(0xFF0F, 0x00);
    handler.requestTimerInterrupt();
    EXPECT_EQ(handler.read(0xFF0F), 0x04);

    handler.write(0xFF0F, 0x00);
    handler.requestSerialInterrupt();
    EXPECT_EQ(handler.read(0xFF0F), 0x08);

    handler.write(0xFF0F, 0x00);
    handler.requestJoypadInterrupt();
    EXPECT_EQ(handler.read(0xFF0F), 0x10);
}

TEST(GbInterruptTest, PendingRequiresEnableMask)
{
    qnemu::GbInterruptHandler handler;
    handler.write(0xFF0F, 0x1F);
    handler.write(0xFFFF, 0x04);

    EXPECT_EQ(handler.pending(), 0x04);

    handler.write(0xFFFF, 0x00);
    EXPECT_EQ(handler.pending(), 0x00);
}

TEST(GbInterruptTest, AcknowledgeClearsRequestBits)
{
    qnemu::GbInterruptHandler handler;
    handler.write(0xFF0F, 0x1F);
    handler.write(0xFFFF, 0x1F);

    handler.acknowledge(0x05);

    EXPECT_EQ(handler.read(0xFF0F), 0x1A);
    EXPECT_EQ(handler.pending(), 0x1A);
}

TEST(GbInterruptTest, WrongAddress)
{
    qnemu::GbInterruptHandler handler;

#ifdef NDEBUG
    EXPECT_EQ(handler.read(0xFF00), 0xFF);
    EXPECT_NO_THROW(handler.write(0xFF00, 0));
#else
    EXPECT_DEATH(handler.read(0xFF00), "");
    EXPECT_DEATH(handler.write(0xFF00, 0), "");
#endif
}

}  // namespace qnemuTest
