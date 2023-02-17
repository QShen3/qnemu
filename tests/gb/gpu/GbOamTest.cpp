/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <random>

#include <gtest/gtest.h>

#include "mock/gb/cpu/MockGbCpu.h"
#include "mock/gb/gpu/MockGbGpu.h"
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
        mockGbGpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbGpu>>();
        gbOam = std::make_unique<qnemu::GbOam>(mockGbCpu, *mockGbGpu);
    }

protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbCpu>> mockGbCpu;
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbGpu>> mockGbGpu;
    std::unique_ptr<qnemu::GbOam> gbOam;
};

TEST_F(GbOamTest, Accept)
{
    for (uint32_t i = 0; i <= qnemu::MemorySpaceSize; i++) {
        if (i >= qnemu::OamStart && i <= qnemu::OamEnd) {
            EXPECT_TRUE(gbOam->accepts(i));
        } else if (i == 0xFF46){
            EXPECT_TRUE(gbOam->accepts(i));
        } else {
            EXPECT_FALSE(gbOam->accepts(i));
        }
    }
}

TEST_F(GbOamTest, ReadAndWriteDataInMode2)
{
    EXPECT_CALL(*mockGbGpu, currentMode()).WillRepeatedly(testing::Return(2));
    EXPECT_CALL(*mockGbGpu, isLcdEnable()).WillRepeatedly(testing::Return(true));
    for (uint32_t i = qnemu::OamStart; i <= qnemu::OamEnd; i++) {
        gbOam->write(i, 0);
        EXPECT_EQ(0xFF, gbOam->read(i));
    }
}

TEST_F(GbOamTest, ReadAndWriteDataInMode3)
{
    EXPECT_CALL(*mockGbGpu, currentMode()).WillRepeatedly(testing::Return(3));
    EXPECT_CALL(*mockGbGpu, isLcdEnable()).WillRepeatedly(testing::Return(true));
    for (uint32_t i = qnemu::OamStart; i <= qnemu::OamEnd; i++) {
        gbOam->write(i, 0);
        EXPECT_EQ(0xFF, gbOam->read(i));
    }
}

TEST_F(GbOamTest, ReadAndWriteData)
{
    EXPECT_CALL(*mockGbGpu, currentMode()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*mockGbGpu, isLcdEnable()).WillRepeatedly(testing::Return(false));
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
