/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <random>

#include <gtest/gtest.h>

#include "mock/gb/cpu/MockGbCpu.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/GbOam.h"

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
        mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
        gbOam = std::make_unique<qnemu::GbOam>(mockGbCpu);
    }

protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbCpu>> mockGbCpu;
    std::unique_ptr<qnemu::GbOam> gbOam;
};

TEST_F(GbOamTest, ReadAndWriteData)
{
    for (uint32_t i = qnemu::OamStart; i <= qnemu::OamEnd; i++) {
        uint8_t value = distrib(gen);
        gbOam->write(i, value);
        EXPECT_EQ(value, gbOam->read(i));
    }

    uint8_t value = distrib(gen);
    gbOam->write(0xFF46, value);
    EXPECT_EQ(value, gbOam->read(0xFF46));
}

}  // namespace qnemuTest
