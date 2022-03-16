/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

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
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbc1.h"

namespace qnemuTest
{

const std::tuple<uint8_t, std::type_index> typesOfMbc[] = {
    {0x0, std::type_index(typeid(qnemu::GbCircuitMbc))},
    {0x8, std::type_index(typeid(qnemu::GbCircuitMbc))},
    {0x9, std::type_index(typeid(qnemu::GbCircuitMbc))},
    {0x1, std::type_index(typeid(qnemu::GbMbc1))},
    {0x2, std::type_index(typeid(qnemu::GbMbc1))},
    {0x3, std::type_index(typeid(qnemu::GbMbc1))},
    // {0x5, std::type_index(typeid(qnemu::GbMbc2))},
    // {0x6, std::type_index(typeid(qnemu::GbMbc2))},
    // {0xF, std::type_index(typeid(qnemu::GbMbc3))},
    // {0x10, std::type_index(typeid(qnemu::GbMbc3))},
    // {0x11, std::type_index(typeid(qnemu::GbMbc3))},
    // {0x12, std::type_index(typeid(qnemu::GbMbc3))},
    // {0x13, std::type_index(typeid(qnemu::GbMbc3))},
    // {0x19, std::type_index(typeid(qnemu::GbMbc5))},
    // {0x1A, std::type_index(typeid(qnemu::GbMbc5))},
    // {0x1B, std::type_index(typeid(qnemu::GbMbc5))},
    // {0x1C, std::type_index(typeid(qnemu::GbMbc5))},
    // {0x1D, std::type_index(typeid(qnemu::GbMbc5))},
    // {0x1E, std::type_index(typeid(qnemu::GbMbc5))},
};

class GbMbcFactoryTest : public testing::TestWithParam<std::tuple<uint8_t, std::type_index>>
{
    public:
        GbMbcFactoryTest() {}

    protected:
        qnemu::GbMbcFactory gbMbcFactory;
};

TEST_P(GbMbcFactoryTest, CreateMbc)
{
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>> romBanks;
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>> ramBanks;
    auto [ type, typeIndex ] = GetParam();
    auto mbc = gbMbcFactory.create(std::move(romBanks), std::move(ramBanks), type);
    auto* mbc_pointer = mbc.get();
    EXPECT_EQ(std::type_index(typeid(*mbc_pointer)), typeIndex);
}

#if defined(__GNUC__) && !defined(__llvm__) && __GNUC__ < 9
INSTANTIATE_TEST_CASE_P(GbMbcFactoryCreate, GbMbcFactoryTest, testing::ValuesIn(typesOfMbc));
#else
INSTANTIATE_TEST_SUITE_P(GbMbcFactoryCreate, GbMbcFactoryTest, testing::ValuesIn(typesOfMbc));
#endif

TEST_F(GbMbcFactoryTest, CreateMbcWithInvalidType)
{
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>> romBanks;
    std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>> ramBanks;
    EXPECT_THROW({
        try {
            gbMbcFactory.create(std::move(romBanks), std::move(ramBanks), 0xFF);
        }
        catch(const std::runtime_error& e) {
            EXPECT_STREQ(e.what(), "This game is not supported!");
            throw;
        }
    }, std::runtime_error);
}

}  // namespace qnemuTest
