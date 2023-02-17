/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <random>

#include <gtest/gtest.h>

#include "mock/gb/cpu/MockGbCpu.h"
#include "mock/gb/gpu/MockGbGpu.h"
#include "qnemu/gb/const.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"

namespace qnemuTest
{

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> distrib(0, 255);

class SpriteAttributeTableTest : public testing::Test
{
public:
    void SetUp() override
    {
        mockGbCpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbCpu>>();
        mockGbGpu = std::make_shared<testing::StrictMock<qnemuMock::MockGbGpu>>();
        spriteAttributeTable = std::make_unique<qnemu::SpriteAttributeTable>(mockGbCpu, *mockGbGpu);
    }

protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbCpu>> mockGbCpu;
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbGpu>> mockGbGpu;
    std::unique_ptr<qnemu::SpriteAttributeTable> spriteAttributeTable;
};

TEST_F(SpriteAttributeTableTest, Accept)
{
    for (uint32_t i = 0; i <= qnemu::MemorySpaceSize; i++) {
        if (i >= qnemu::SpriteAttributeTableStart && i <= qnemu::SpriteAttributeTableEnd) {
            EXPECT_TRUE(spriteAttributeTable->accepts(i));
        } else if (i == 0xFF46){
            EXPECT_TRUE(spriteAttributeTable->accepts(i));
        } else {
            EXPECT_FALSE(spriteAttributeTable->accepts(i));
        }
    }
}

TEST_F(SpriteAttributeTableTest, ReadAndWriteDataInMode2)
{
    EXPECT_CALL(*mockGbGpu, currentMode()).WillRepeatedly(testing::Return(2));
    EXPECT_CALL(*mockGbGpu, isLcdEnable()).WillRepeatedly(testing::Return(true));
    for (uint32_t i = qnemu::SpriteAttributeTableStart; i <= qnemu::SpriteAttributeTableEnd; i++) {
        spriteAttributeTable->write(i, 0);
        EXPECT_EQ(0xFF, spriteAttributeTable->read(i));
    }
}

TEST_F(SpriteAttributeTableTest, ReadAndWriteDataInMode3)
{
    EXPECT_CALL(*mockGbGpu, currentMode()).WillRepeatedly(testing::Return(3));
    EXPECT_CALL(*mockGbGpu, isLcdEnable()).WillRepeatedly(testing::Return(true));
    for (uint32_t i = qnemu::SpriteAttributeTableStart; i <= qnemu::SpriteAttributeTableEnd; i++) {
        spriteAttributeTable->write(i, 0);
        EXPECT_EQ(0xFF, spriteAttributeTable->read(i));
    }
}

TEST_F(SpriteAttributeTableTest, ReadAndWriteData)
{
    EXPECT_CALL(*mockGbGpu, currentMode()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(*mockGbGpu, isLcdEnable()).WillRepeatedly(testing::Return(false));
    for (uint32_t i = qnemu::SpriteAttributeTableStart; i <= qnemu::SpriteAttributeTableEnd; i++) {
        uint8_t value = distrib(gen);
        spriteAttributeTable->write(i, value);
        EXPECT_EQ(value, spriteAttributeTable->read(i));
    }

    uint8_t value = distrib(gen);
    spriteAttributeTable->write(0xFF46, value);
    EXPECT_EQ(value, spriteAttributeTable->read(0xFF46));
}

}  // namespace qnemuTest
