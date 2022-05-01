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

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbCircuitMbcTest, ReadFromRom)
{
    std::array<uint8_t, qnemu::GbMbcInterface::romBankSize> data0;
    std::array<uint8_t, qnemu::GbMbcInterface::romBankSize> data1;

    for (auto& data : data0) {
        data = distrib(gen);
    }
    for (auto& data : data1) {
        data = distrib(gen);
    }

    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>> romBanks(2);
    std::copy(data0.begin(), data0.end(), romBanks.at(0).begin());
    std::copy(data1.begin(), data1.end(), romBanks.at(1).begin());
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>> ramBanks(1);

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 0);

    for (uint16_t i = 0; i < qnemu::GbMbcInterface::romBankSize; i++) {
        EXPECT_EQ(gbCircuitMbc.read(i), data0.at(i));
        EXPECT_EQ(gbCircuitMbc.read(i + qnemu::GbMbcInterface::romBankSize), data1.at(i));
    }
}

#ifdef NDEBUG
TEST(GbCircuitMbcTest, ReadFromNoneExistRam)
{
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>> ramBanks(1);

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 0);

    for (uint16_t i = 0; i < qnemu::GbMbcInterface::ramBankSize; i++) {
        EXPECT_EQ(0xFF, gbCircuitMbc.read(i + 0xA000));
    }
}
#endif

TEST(GbCircuitMbcTest, ReadFromRam)
{
    std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize> data0;

    for (auto& data : data0) {
        data = distrib(gen);
    }

    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>> ramBanks(1);
    std::copy(data0.begin(), data0.end(), ramBanks.at(0).begin());

    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 1);

    for (uint16_t i = 0; i < qnemu::GbMbcInterface::ramBankSize; i++) {
        EXPECT_EQ(gbCircuitMbc.read(i + 0xA000), data0.at(i));
    }
}

TEST(GbCircuitMbcTest, WriteToRam)
{
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>> ramBanks(1);
    qnemu::GbCircuitMbc gbCircuitMbc(std::move(romBanks), std::move(ramBanks), 1);

    for (uint16_t i = 0; i < qnemu::GbMbcInterface::ramBankSize; i++) {
        uint8_t value = distrib(gen);
        gbCircuitMbc.write(i + 0xA000, value);
        EXPECT_EQ(gbCircuitMbc.read(i + 0xA000), value);
    }
}

}  // namespace qnemuTest