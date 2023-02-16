/* 
 *  Copyright [2021] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#endif
#include <gtest/gtest.h>
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "qnemu/gb/cartridge/mbc/GbCircuitMbc.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbCircuitMbcTest, ReadFromRom)
{
    std::array<uint8_t, qnemu::RomBankSize> data0;
    std::array<uint8_t, qnemu::RomBankSize> data1;

    for (auto& data : data0) {
        data = distrib(gen);
    }
    for (auto& data : data1) {
        data = distrib(gen);
    }

    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::copy(data0.begin(), data0.end(), romBanks.at(0).begin());
    std::copy(data1.begin(), data1.end(), romBanks.at(1).begin());
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 0);

    for (uint16_t i = 0; i < qnemu::RomBankSize; i++) {
        EXPECT_EQ(gbCircuitMbc.read(i), data0.at(i));
        EXPECT_EQ(gbCircuitMbc.read(i + qnemu::RomBankSize), data1.at(i));
    }
}

#ifdef NDEBUG
TEST(GbCircuitMbcTest, ReadFromNoneExistRam)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 0);

    for (uint16_t i = 0; i < qnemu::RamBankSize; i++) {
        EXPECT_EQ(0xFF, gbCircuitMbc.read(i + qnemu::ExternalRamStart));
    }
}

TEST(GbCircuitMbcTest, ReadFromWrongAddress)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 0);
    for (uint32_t i = 0; i <= 0xFFFF; i++) {
        if (gbCircuitMbc.accepts(i) == false) {
            EXPECT_EQ(0xFF, gbCircuitMbc.read(i));
        }
    }
}
#endif

TEST(GbCircuitMbcTest, ReadFromRam)
{
    std::array<uint8_t, qnemu::RamBankSize> data0;

    for (auto& data : data0) {
        data = distrib(gen);
    }

    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);
    std::copy(data0.begin(), data0.end(), ramBanks.at(0).begin());

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 1);

    for (uint16_t i = 0; i < qnemu::RamBankSize; i++) {
        EXPECT_EQ(gbCircuitMbc.read(i + qnemu::ExternalRamStart), data0.at(i));
    }
}

TEST(GbCircuitMbcTest, WriteToRam)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);
    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 1);

    for (uint16_t i = 0; i < qnemu::RamBankSize; i++) {
        uint8_t value = distrib(gen);
        gbCircuitMbc.write(i + qnemu::ExternalRamStart, value);
        EXPECT_EQ(gbCircuitMbc.read(i + qnemu::ExternalRamStart), value);
    }
}

TEST(GbCircuitMbcTest, Reset)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 0);

    EXPECT_NO_THROW(gbCircuitMbc.reset());
}

}  // namespace qnemuTest