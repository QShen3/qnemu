/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <random>

#include <gtest/gtest.h>

#include "mock/gb/cartridge/MockGbCartridge.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbOam.h"
#include "qnemu/gb/gpu/GbVideoRam.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

class GbOamTest : public testing::Test
{
public:
    void SetUp() override
    {
        // oam = std::make_unique<qnemu::GbOam>();
    }

protected:
    testing::StrictMock<qnemuMock::MockGbCartridge> mockCartridge;
    std::unique_ptr<qnemu::GbOam> oam;
};

TEST_F(GbOamTest, ReadAndWriteData)
{
    // for (uint32_t i = qnemu::OamStart; i <= qnemu::OamEnd; i++) {
    //     uint8_t value = distrib(gen);
    //     oam->write(i, value);
    //     EXPECT_EQ(value, oam->read(i));
    // }

    // uint8_t value = distrib(gen);
    // oam->write(0xFF46, value);
    // EXPECT_EQ(value, oam->read(0xFF46));
}

}  // namespace qnemuTest
