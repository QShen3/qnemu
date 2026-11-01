/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "mock/gb/interrupt/MockGbInterruptHandler.h"
#include "qnemu/gb/serial/GbLocalSerialLink.h"
#include "qnemu/gb/serial/GbSerial.h"

namespace qnemuTest
{

namespace
{

using StrictInterrupt = testing::StrictMock<qnemuMock::MockGbInterruptHanlder>;

constexpr uint16_t serialData = 0xFF01;
constexpr uint16_t serialControl = 0xFF02;

struct SerialDevice
{
    std::shared_ptr<StrictInterrupt> interrupt = std::make_shared<StrictInterrupt>();
    std::unique_ptr<qnemu::GbSerial> serial;
};

SerialDevice makeDevice(bool cgb, const std::shared_ptr<qnemu::GbSerialLink>& link = nullptr)
{
    SerialDevice device;
    device.serial = std::make_unique<qnemu::GbSerial>(device.interrupt, link, cgb);
    return device;
}

void step(qnemu::GbSerial& serial, int cycles)
{
    for (int cycle = 0; cycle < cycles; ++cycle) {
        serial.step();
    }
}

}  // namespace

TEST(GbSerialTest, ResetValuesMatchHardware)
{
    auto dmg = makeDevice(false);
    EXPECT_EQ(dmg.serial->read(serialData), 0x00);
    EXPECT_EQ(dmg.serial->read(serialControl), 0x7E);

    auto cgb = makeDevice(true);
    EXPECT_EQ(cgb.serial->read(serialData), 0x00);
    EXPECT_EQ(cgb.serial->read(serialControl), 0x7F);

    dmg.serial->write(serialData, 0xA5);
    EXPECT_EQ(dmg.serial->read(serialData), 0xA5);

    // Bit 1 is not writable on DMG; the fast-clock bit is CGB-only.
    dmg.serial->write(serialControl, 0x02);
    EXPECT_EQ(dmg.serial->read(serialControl), 0x7E);

    cgb.serial->write(serialControl, 0x02);
    EXPECT_EQ(cgb.serial->read(serialControl), 0x7E);
}

TEST(GbSerialTest, InternalClockWithoutLinkReceivesFF)
{
    auto device = makeDevice(false);
    device.serial->write(serialData, 0x12);
    device.serial->write(serialControl, 0x81);

    EXPECT_TRUE(device.serial->isTransferActive());
    EXPECT_EQ(device.serial->read(serialControl), 0xFF);

    step(*device.serial, 4095);
    EXPECT_TRUE(device.serial->isTransferActive());

    EXPECT_CALL(*device.interrupt, requestSerialInterrupt()).Times(1);
    device.serial->step();

    EXPECT_FALSE(device.serial->isTransferActive());
    EXPECT_EQ(device.serial->read(serialData), 0xFF);
    EXPECT_EQ(device.serial->read(serialControl), 0x7F);
}

TEST(GbSerialTest, ExternalClockWaitsForThePeer)
{
    auto device = makeDevice(false);
    device.serial->write(serialData, 0x12);
    device.serial->write(serialControl, 0x80);

    // An externally clocked transfer never completes on its own.
    step(*device.serial, 20000);
    EXPECT_TRUE(device.serial->isTransferActive());
    EXPECT_EQ(device.serial->read(serialControl), 0xFE);
}

TEST(GbSerialTest, TwoDevicesExchangeOneByte)
{
    const auto [masterLink, slaveLink] = qnemu::createLocalSerialLinkPair();
    auto master = makeDevice(false, masterLink);
    auto slave = makeDevice(false, slaveLink);

    slave.serial->write(serialData, 0x34);
    slave.serial->write(serialControl, 0x80);
    master.serial->write(serialData, 0x12);
    master.serial->write(serialControl, 0x81);

    EXPECT_CALL(*master.interrupt, requestSerialInterrupt()).Times(1);
    step(*master.serial, 4096);

    EXPECT_FALSE(master.serial->isTransferActive());
    EXPECT_EQ(master.serial->read(serialData), 0x34);
    EXPECT_EQ(master.serial->read(serialControl), 0x7F);

    // The slave completes when its own serial unit next runs.
    EXPECT_TRUE(slave.serial->isTransferActive());
    EXPECT_CALL(*slave.interrupt, requestSerialInterrupt()).Times(1);
    slave.serial->step();

    EXPECT_FALSE(slave.serial->isTransferActive());
    EXPECT_EQ(slave.serial->read(serialData), 0x12);
    EXPECT_EQ(slave.serial->read(serialControl), 0x7E);
}

TEST(GbSerialTest, MasterCapturesPeerShiftRegisterAtStart)
{
    const auto [masterLink, slaveLink] = qnemu::createLocalSerialLinkPair();
    auto master = makeDevice(false, masterLink);
    auto slave = makeDevice(false, slaveLink);

    // The slave has not armed SC yet, but its SB is still shifted out.
    slave.serial->write(serialData, 0x55);
    master.serial->write(serialData, 0x12);
    master.serial->write(serialControl, 0x81);

    EXPECT_CALL(*master.interrupt, requestSerialInterrupt()).Times(1);
    step(*master.serial, 4096);
    EXPECT_EQ(master.serial->read(serialData), 0x55);

    // A passive endpoint still receives the byte and a serial interrupt.
    EXPECT_CALL(*slave.interrupt, requestSerialInterrupt()).Times(1);
    slave.serial->step();
    EXPECT_EQ(slave.serial->read(serialData), 0x12);
    EXPECT_EQ(slave.serial->read(serialControl), 0x7E);
}

TEST(GbSerialTest, BothInternalClockEndpointsStillExchange)
{
    const auto [firstLink, secondLink] = qnemu::createLocalSerialLinkPair();
    auto first = makeDevice(false, firstLink);
    auto second = makeDevice(false, secondLink);

    first.serial->write(serialData, 0x11);
    second.serial->write(serialData, 0x22);
    first.serial->write(serialControl, 0x81);
    second.serial->write(serialControl, 0x81);

    // The first endpoint owns the clock; the second one waits.
    EXPECT_CALL(*first.interrupt, requestSerialInterrupt()).Times(1);
    step(*first.serial, 4096);
    EXPECT_EQ(first.serial->read(serialData), 0x22);

    EXPECT_CALL(*second.interrupt, requestSerialInterrupt()).Times(1);
    second.serial->step();
    EXPECT_EQ(second.serial->read(serialData), 0x11);
}

TEST(GbSerialTest, CgbFastClockAndDoubleSpeed)
{
    auto cgb = makeDevice(true);
    cgb.serial->write(serialData, 0x5A);
    cgb.serial->write(serialControl, 0x83);

    EXPECT_CALL(*cgb.interrupt, requestSerialInterrupt()).Times(1);
    step(*cgb.serial, 128);
    EXPECT_FALSE(cgb.serial->isTransferActive());

    cgb.serial->setDoubleSpeed(true);
    cgb.serial->write(serialData, 0xA5);
    cgb.serial->write(serialControl, 0x83);

    EXPECT_CALL(*cgb.interrupt, requestSerialInterrupt()).Times(1);
    step(*cgb.serial, 64);
    EXPECT_FALSE(cgb.serial->isTransferActive());
}

TEST(GbSerialTest, DoubleSpeedHalvesTheNormalClockPeriod)
{
    auto cgb = makeDevice(true);
    cgb.serial->setDoubleSpeed(true);
    cgb.serial->write(serialData, 0x5A);
    cgb.serial->write(serialControl, 0x81);

    step(*cgb.serial, 2047);
    EXPECT_TRUE(cgb.serial->isTransferActive());

    EXPECT_CALL(*cgb.interrupt, requestSerialInterrupt()).Times(1);
    cgb.serial->step();
    EXPECT_FALSE(cgb.serial->isTransferActive());
}

TEST(GbSerialTest, ClearingStartBitAbortsTransfer)
{
    auto device = makeDevice(false);
    device.serial->write(serialData, 0x12);
    device.serial->write(serialControl, 0x81);
    step(*device.serial, 100);

    device.serial->write(serialControl, 0x01);
    EXPECT_FALSE(device.serial->isTransferActive());
    EXPECT_EQ(device.serial->read(serialControl), 0x7F);

    step(*device.serial, 5000);
    EXPECT_EQ(device.serial->read(serialData), 0x12);
}

TEST(GbSerialTest, ResetDropsTheTransfer)
{
    auto device = makeDevice(false);
    device.serial->write(serialData, 0x12);
    device.serial->write(serialControl, 0x81);
    step(*device.serial, 100);

    device.serial->reset();
    EXPECT_FALSE(device.serial->isTransferActive());
    EXPECT_EQ(device.serial->read(serialData), 0x00);
    EXPECT_EQ(device.serial->read(serialControl), 0x7E);
}

TEST(GbSerialTest, CgbModeSwitchHidesTheFastClock)
{
    auto cgb = makeDevice(true);
    cgb.serial->setCgbMode(false);
    EXPECT_EQ(cgb.serial->read(serialControl), 0x7F);

    cgb.serial->write(serialControl, 0x02);
    EXPECT_EQ(cgb.serial->read(serialControl), 0x7E);

    cgb.serial->setCgbMode(true);
    EXPECT_EQ(cgb.serial->read(serialControl), 0x7C);

    cgb.serial->write(serialControl, 0x02);
    EXPECT_EQ(cgb.serial->read(serialControl), 0x7E);
}

}  // namespace qnemuTest
