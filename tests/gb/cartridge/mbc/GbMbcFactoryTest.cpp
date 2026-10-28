/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <utility>
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
#include "qnemu/gb/cartridge/mbc/GbHuc1.h"
#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/cartridge/mbc/GbMbc1M.h"
#include "qnemu/gb/cartridge/mbc/GbMbc2.h"
#include "qnemu/gb/cartridge/mbc/GbMbc3.h"
#include "qnemu/gb/cartridge/mbc/GbMbc30.h"
#include "qnemu/gb/cartridge/mbc/GbMbc5.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/const.h"

namespace qnemuTest
{

const std::tuple<uint8_t, size_t> typesOfMbc[] = {
    {0x0, typeid(qnemu::GbCircuitMbc).hash_code()},
    {0x8, typeid(qnemu::GbCircuitMbc).hash_code()},
    {0x9, typeid(qnemu::GbCircuitMbc).hash_code()},
    {0x1, typeid(qnemu::GbMbc1).hash_code()},
    {0x2, typeid(qnemu::GbMbc1).hash_code()},
    {0x3, typeid(qnemu::GbMbc1).hash_code()},
    {0x5, typeid(qnemu::GbMbc2).hash_code()},
    {0x6, typeid(qnemu::GbMbc2).hash_code()},
    {0xF, typeid(qnemu::GbMbc3).hash_code()},
    {0x10, typeid(qnemu::GbMbc3).hash_code()},
    {0x11, typeid(qnemu::GbMbc3).hash_code()},
    {0x12, typeid(qnemu::GbMbc3).hash_code()},
    {0x13, typeid(qnemu::GbMbc3).hash_code()},
    {0x19, typeid(qnemu::GbMbc5).hash_code()},
    {0x1A, typeid(qnemu::GbMbc5).hash_code()},
    {0x1B, typeid(qnemu::GbMbc5).hash_code()},
    {0x1C, typeid(qnemu::GbMbc5).hash_code()},
    {0x1D, typeid(qnemu::GbMbc5).hash_code()},
    {0x1E, typeid(qnemu::GbMbc5).hash_code()},
    {0xFF, typeid(qnemu::GbHuc1).hash_code()},
};

class GbMbcFactoryTest : public testing::TestWithParam<std::tuple<uint8_t, size_t>>
{
public:
    GbMbcFactoryTest() {}
};

TEST_P(GbMbcFactoryTest, CreateMbc)
{
    const qnemu::GbMbcFactory gbMbcFactory;
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks;
    auto [ type, typeHash ] = GetParam();
    auto mbc = gbMbcFactory.create(std::move(romBanks), std::move(ramBanks), type);
    const auto* mbc_pointer = mbc.get();
    EXPECT_EQ(typeid(*mbc_pointer).hash_code(), typeHash);
}

#if defined(__GNUC__) && !defined(__llvm__) && __GNUC__ < 9
INSTANTIATE_TEST_CASE_P(GbMbcFactoryCreate, GbMbcFactoryTest, testing::ValuesIn(typesOfMbc));
#else
INSTANTIATE_TEST_SUITE_P(GbMbcFactoryCreate, GbMbcFactoryTest, testing::ValuesIn(typesOfMbc));
#endif

TEST_F(GbMbcFactoryTest, DetectsMbc1Multicart)
{
    const qnemu::GbMbcFactory gbMbcFactory;
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(64);
    const std::array<uint8_t, 8> logoPrefix = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    };
    std::copy(logoPrefix.begin(), logoPrefix.end(), romBanks.at(0x10).begin() + 0x104);

    auto mbc = gbMbcFactory.create(
        std::move(romBanks), std::vector<std::array<uint8_t, qnemu::RamBankSize>>(), 0x1);
    EXPECT_NE(dynamic_cast<qnemu::GbMbc1M*>(mbc.get()), nullptr);
}

TEST_F(GbMbcFactoryTest, DetectsMbc30)
{
    const qnemu::GbMbcFactory gbMbcFactory;
    std::vector<std::array<uint8_t, qnemu::RomBankSize>> romBanks(256);
    std::vector<std::array<uint8_t, qnemu::RamBankSize>> ramBanks(8);

    auto mbc = gbMbcFactory.create(std::move(romBanks), std::move(ramBanks), 0x10);
    EXPECT_NE(dynamic_cast<qnemu::GbMbc30*>(mbc.get()), nullptr);
}

TEST_F(GbMbcFactoryTest, CreateMbcWithInvalidType)
{
    const qnemu::GbMbcFactory gbMbcFactory;
    EXPECT_THROW({
        try {
            gbMbcFactory.create(std::vector<std::array<uint8_t, qnemu::RomBankSize>>(), std::vector<std::array<uint8_t, qnemu::RamBankSize>>(), 0x20);
        }
        catch(const std::runtime_error& e) {
            EXPECT_STREQ(e.what(), "This game is not supported!");
            throw;
        }
    }, std::runtime_error);
}

}  // namespace qnemuTest
