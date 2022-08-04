/* 
 *  Copyright [2021] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
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

#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

TEST(GbMbc1Test, ReadFrom1BankRom)
{
    std::array<uint8_t, qnemu::RomBankSize> data;

    for (auto& value : data) {
        value = distrib(gen);
    }

    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(1);
    std::copy(data.begin(), data.end(), romBanks.at(0).begin());
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);

    qnemu::GbMbc1 gbMbc1(std::move(romBanks), std::move(ramBanks), 0);
    gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 0);

    for (uint16_t i = 0; i < qnemu::RomBankSize; i++) {
        EXPECT_EQ(gbMbc1.read(i), data.at(i));
    }

    gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 1);
    gbMbc1.write(qnemu::GbMbc1::ramBankNumberAddress, 1);
    EXPECT_THROW(gbMbc1.read(0x4000), std::out_of_range);
}

TEST(GbMbc1Test, ReadFromRom)
{
    auto romDataBanks = std::make_unique<std::array<std::array<uint8_t, qnemu::RomBankSize>, 128>>();
    for (auto& romData : *romDataBanks) {
        for (auto& data : romData) {
            data = distrib(gen);
        }
    }

    for (size_t size = 2; size <= 128; size = size * 2) {
        std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(size);
        std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);
        for (size_t i = 0; i < romBanks.size(); i++) {
            std::copy(romDataBanks->at(i).begin(), romDataBanks->at(i).end(), romBanks.at(i).begin());
        }

        qnemu::GbMbc1 gbMbc1(std::move(romBanks), std::move(ramBanks), 0);
        gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 0);
        for (uint16_t i = 0; i < qnemu::RomBankSize; i++) {
            EXPECT_EQ(gbMbc1.read(i), romDataBanks->at(0).at(i));
        }

        gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 1);
        for (uint16_t i = 0x20; i < size; i = i + 0x20) {
            gbMbc1.write(qnemu::GbMbc1::ramBankNumberAddress, static_cast<uint8_t>(i >> 5));
            for (uint16_t j = 0; j < qnemu::RomBankSize; j++) {
                EXPECT_EQ(gbMbc1.read(j), romDataBanks->at(i).at(j));
            }
        }

        for (uint16_t i = qnemu::RomBankSize; i < qnemu::RomBankSize * 2; i++) {
            for (uint8_t j = 1; j < (size & 32); j++) {
                gbMbc1.write(qnemu::GbMbc1::romBankNumberAddress, (j & 0b11111));
                if (size < 64) {
                    EXPECT_EQ(gbMbc1.read(i), romDataBanks->at(j).at(i - 0x4000));
                }
                else {
                    for (uint8_t k = 1; k < (size >> 5); k++) {
                        gbMbc1.write(qnemu::GbMbc1::ramBankNumberAddress, k);
                        EXPECT_EQ(gbMbc1.read(i), romDataBanks->at((k << 5) | j).at(i - 0x4000));
                    }
                }
            }
        }
    }
}

#ifdef NDEBUG
TEST(GbMbc1Test, ReadWhenRamDisabled)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(1);

    qnemu::GbMbc1 gbMbc1(std::move(romBanks), std::move(ramBanks), 0);

    gbMbc1.write(qnemu::GbMbc1::ramEnableAddress, 0);
    for (uint16_t i = 0; i < qnemu::RamBankSize; i++) {
        EXPECT_EQ(0xFF, gbMbc1.read(i + 0xA000));
    }
}
#endif

TEST(GbMbc1Test, ReadFromRam)
{
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramDataBanks(4);

    for (auto& ramData : ramDataBanks) {
        for (auto& data : ramData) {
            data = distrib(gen);
        }
    }

    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(4);
    for (size_t i = 0; i < ramBanks.size(); i++) {
        std::copy(ramDataBanks.at(i).begin(), ramDataBanks.at(i).end(), ramBanks.at(i).begin());
    }

    qnemu::GbMbc1 gbMbc1(std::move(romBanks), std::move(ramBanks), 1);
    gbMbc1.write(qnemu::GbMbc1::ramEnableAddress, 0xA);
 
    gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 0);
    for (uint16_t i = 0; i < qnemu::RamBankSize; i++) {
        EXPECT_EQ(gbMbc1.read(i + 0xA000), ramDataBanks.at(0).at(i));
    }

    gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 1);
    for (uint8_t i = 0; i < 4; i++) {
        gbMbc1.write(qnemu::GbMbc1::ramBankNumberAddress, i);
        for (uint16_t j = 0; j < qnemu::RamBankSize; j++) {
            EXPECT_EQ(gbMbc1.read(j + 0xA000), ramDataBanks.at(i).at(j));
        }
    }
}

TEST(GbMbc1Test, WriteToRam)
{
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(2);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(4);
    qnemu::GbMbc1 gbMbc1(std::move(romBanks), std::move(ramBanks), 1);
    gbMbc1.write(qnemu::GbMbc1::ramEnableAddress, 0xA);

    gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 0);
    for (uint16_t i = 0; i < qnemu::RamBankSize; i++) {
        uint8_t value = distrib(gen);
        gbMbc1.write(i + 0xA000, value);
        EXPECT_EQ(gbMbc1.read(i + 0xA000), value);
    }

    gbMbc1.write(qnemu::GbMbc1::bankingModeSelectAddress, 1);
    for (uint8_t i = 0; i < 4; i++) {
        gbMbc1.write(qnemu::GbMbc1::ramBankNumberAddress, i);
        for (uint16_t j = 0; j < qnemu::RamBankSize; j++) {
            uint8_t value = distrib(gen);
            gbMbc1.write(j + 0xA000, value);
            EXPECT_EQ(gbMbc1.read(j + 0xA000), value);
        }
    }
}

}  // namespace qnemuTest