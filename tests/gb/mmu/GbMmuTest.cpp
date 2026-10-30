/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "qnemu/gb/apu/GbApu.h"
#include "qnemu/gb/apu/GbChannel1.h"
#include "qnemu/gb/apu/GbChannel2.h"
#include "qnemu/gb/apu/GbChannel3.h"
#include "qnemu/gb/apu/GbChannel4.h"
#include "qnemu/gb/apu/GbChannelInterface.h"
#include "qnemu/gb/cartridge/GbCartridgeInterface.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbGpuInterface.h"
#include "qnemu/gb/interrupt/GbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"
#include "qnemu/gb/memory/GbHighRam.h"
#include "qnemu/gb/memory/GbWorkRam.h"
#include "qnemu/gb/mmu/GbMmu.h"
#include "qnemu/gb/timer/GbTimer.h"
#include "tests/support/NullSound.h"

namespace qnemuTest
{

namespace
{

class FakeCartridge : public qnemu::GbCartridgeInterface
{
public:
    uint8_t read(uint16_t address) const override
    {
        if (address >= qnemu::ExternalRamStart && address <= qnemu::ExternalRamEnd) {
            return ram.at(address - qnemu::ExternalRamStart);
        }
        return static_cast<uint8_t>(address & 0xFF);
    }

    void write(uint16_t address, const uint8_t& value) override
    {
        if (address >= qnemu::ExternalRamStart && address <= qnemu::ExternalRamEnd) {
            ram.at(address - qnemu::ExternalRamStart) = value;
        }
    }

    void step() override { ++steps; }
    void reset() override { ++resets; }
    void load(const char*) override { ++loads; }
    bool isGbcCartridge() const override { return false; }
    qnemu::GbCartridgeHeader cartridgeHeader() const override { return {}; }

    std::array<uint8_t, 0x2000> ram{};
    int steps = 0;
    int resets = 0;
    int loads = 0;
};

class FakeGpu : public qnemu::GbGpuInterface
{
public:
    uint8_t read(uint16_t address) const override
    {
        if (address >= qnemu::VideoRamStart && address <= qnemu::VideoRamEnd) {
            return vram.at(address - qnemu::VideoRamStart);
        }
        if (address >= qnemu::OamStart && address <= qnemu::OamEnd) {
            return oam.at(address - qnemu::OamStart);
        }
        if (address >= 0xFF40 && address <= 0xFF4B) {
            return registers.at(address - 0xFF40);
        }
        if (address == 0xFF4F) {
            return vramBank;
        }
        if (address >= 0xFF51 && address <= 0xFF55) {
            return hdma.at(address - 0xFF51);
        }
        if (address >= 0xFF68 && address <= 0xFF6C) {
            return cgb.at(address - 0xFF68);
        }
        return 0xFF;
    }

    void write(uint16_t address, const uint8_t& value) override
    {
        if (address >= qnemu::VideoRamStart && address <= qnemu::VideoRamEnd) {
            vram.at(address - qnemu::VideoRamStart) = value;
        } else if (address >= qnemu::OamStart && address <= qnemu::OamEnd) {
            oam.at(address - qnemu::OamStart) = value;
        } else if (address >= 0xFF40 && address <= 0xFF4B) {
            registers.at(address - 0xFF40) = value;
        } else if (address == 0xFF4F) {
            vramBank = value;
        } else if (address >= 0xFF51 && address <= 0xFF55) {
            hdma.at(address - 0xFF51) = value;
        } else if (address >= 0xFF68 && address <= 0xFF6C) {
            cgb.at(address - 0xFF68) = value;
        }
    }

    void step() override { ++steps; }
    void reset() override { ++resets; }
    uint8_t currentMode() const override { return 2; }
    bool isLcdEnable() const override { return true; }

    std::array<uint8_t, 0x2000> vram{};
    std::array<uint8_t, 0xA0> oam{};
    std::array<uint8_t, 0x0C> registers{};
    std::array<uint8_t, 0x05> hdma{};
    std::array<uint8_t, 0x05> cgb{};
    uint8_t vramBank = 0;
    int steps = 0;
    int resets = 0;
};

std::array<std::unique_ptr<qnemu::GbChannelInterface>, 4> makeChannels()
{
    return {
        std::make_unique<qnemu::GbChannel1>(),
        std::make_unique<qnemu::GbChannel2>(),
        std::make_unique<qnemu::GbChannel3>(),
        std::make_unique<qnemu::GbChannel4>(),
    };
}

}  // namespace

class GbMmuTest : public testing::Test
{
protected:
    void SetUp() override
    {
        cartridge = std::make_shared<FakeCartridge>();
        auto gpu = std::make_unique<FakeGpu>();
        gpuPointer = gpu.get();
        auto apu = std::make_unique<qnemu::GbApu>(
            std::make_unique<qnemuTest::NullSound>(), makeChannels(), false);
        auto highRam = std::make_unique<qnemu::GbHighRam>();
        auto joypad = std::make_unique<qnemu::GbJoypad>(interruptHandler);
        auto workRam = std::make_unique<qnemu::GbWorkRam>();
        auto timer = std::make_unique<qnemu::GbTimer>(interruptHandler);

        mmu = std::make_unique<qnemu::GbMmu>(
            std::move(apu), cartridge, std::move(gpu), std::move(highRam),
            interruptHandler, std::move(joypad), std::move(workRam), std::move(timer));
        mmu->setCgb(true);
    }

    std::shared_ptr<FakeCartridge> cartridge;
    FakeGpu* gpuPointer = nullptr;
    std::shared_ptr<qnemu::GbInterruptHandler> interruptHandler =
        std::make_shared<qnemu::GbInterruptHandler>();
    std::unique_ptr<qnemu::GbMmu> mmu;
};

TEST_F(GbMmuTest, RoutesRomVramExternalRamAndOam)
{
    EXPECT_EQ(mmu->read(0x0000), 0x00);
    EXPECT_EQ(mmu->read(0x1234), 0x34);
    EXPECT_EQ(mmu->read(0x7FFF), 0xFF);

    mmu->write(0x8000, 0x5A);
    EXPECT_EQ(mmu->read(0x8000), 0x5A);

    mmu->write(0xA123, 0x6B);
    EXPECT_EQ(mmu->read(0xA123), 0x6B);
    EXPECT_EQ(cartridge->ram.at(0x123), 0x6B);

    mmu->write(0xFE00, 0x7C);
    EXPECT_EQ(mmu->read(0xFE00), 0x7C);
}

TEST_F(GbMmuTest, RoutesGpuRegisters)
{
    mmu->write(0xFF47, 0xE4);
    EXPECT_EQ(mmu->read(0xFF47), 0xE4);

    mmu->write(0xFF4F, 0x01);
    EXPECT_EQ(mmu->read(0xFF4F), 0x01);

    mmu->write(0xFF51, 0x12);
    EXPECT_EQ(mmu->read(0xFF51), 0x12);

    mmu->write(0xFF68, 0x80);
    EXPECT_EQ(mmu->read(0xFF68), 0x80);
}

TEST_F(GbMmuTest, RoutesWorkRamEchoAndHighRam)
{
    mmu->write(0xFF70, 0x03);
    EXPECT_EQ(mmu->read(0xFF70), 0xFB);

    mmu->write(0xD000, 0x11);
    mmu->write(0xFF70, 0x04);
    EXPECT_EQ(mmu->read(0xD000), 0x00);
    mmu->write(0xFF70, 0x03);
    EXPECT_EQ(mmu->read(0xD000), 0x11);

    mmu->write(0xC000, 0x22);
    EXPECT_EQ(mmu->read(0xE000), 0x22);

    mmu->write(0xFF80, 0x33);
    EXPECT_EQ(mmu->read(0xFF80), 0x33);
    mmu->write(0xFFFE, 0x44);
    EXPECT_EQ(mmu->read(0xFFFE), 0x44);
}

TEST_F(GbMmuTest, RoutesJoypadTimerApuAndInterrupts)
{
    mmu->write(0xFF00, 0xF0);
    EXPECT_EQ(mmu->read(0xFF00), 0xFF);

    mmu->write(0xFF06, 0x42);
    EXPECT_EQ(mmu->read(0xFF06), 0x42);

    mmu->write(0xFF24, 0x77);
    EXPECT_EQ(mmu->read(0xFF24), 0x77);

    mmu->write(0xFFFF, 0x1F);
    EXPECT_EQ(mmu->read(0xFFFF), 0x1F);

    mmu->write(0xFF0F, 0x0F);
    EXPECT_EQ(mmu->read(0xFF0F), 0xEF);
}

TEST_F(GbMmuTest, UnmappedReadsReturnFF)
{
    EXPECT_EQ(mmu->read(0xFF03), 0xFF);
    EXPECT_EQ(mmu->read(0xFEA0), 0xFF);
}

TEST_F(GbMmuTest, StepsAndResetsDevices)
{
    mmu->step();
    EXPECT_EQ(cartridge->steps, 1);
    EXPECT_EQ(gpuPointer->steps, 1);

    mmu->write(0xFF70, 0x03);
    mmu->reset();
    EXPECT_EQ(cartridge->resets, 1);
    EXPECT_EQ(gpuPointer->resets, 1);
    EXPECT_EQ(mmu->read(0xFF70), 0xF8);
}

}  // namespace qnemuTest
