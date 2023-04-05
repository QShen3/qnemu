/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <functional>
#include <memory>

#include <QtCore/qnamespace.h>
#include <gtest/gtest.h>

#include "mock/display/MockDisplay.h"
#include "mock/gb/interrupt/MockGbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"

namespace qnemuTest
{

class GbJoypadTest : public testing::Test
{
public:
    void SetUp() override
    {
        mockDisplay = std::make_shared<testing::StrictMock<qnemuMock::MockDisplay>>();
        mockInterruptHandler = std::make_shared<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>>();

        EXPECT_CALL(*mockDisplay, setKeyPressCallback(testing::_)).WillOnce([this](std::function<void(int)> processKeyPressEvent){
            keyPress = processKeyPressEvent;
        });
        EXPECT_CALL(*mockDisplay, setKeyReleaseCallback(testing::_)).WillOnce([this](std::function<void(int)> processKeyReleaseEvent){
            keyRelease = processKeyReleaseEvent;
        });
        gbJoypad = std::make_unique<qnemu::GbJoypad>(mockDisplay, mockInterruptHandler);
    }

protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockDisplay>> mockDisplay;
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>> mockInterruptHandler;
    std::unique_ptr<qnemu::GbJoypad> gbJoypad;
    std::function<void(int)> keyPress;
    std::function<void(int)> keyRelease;
};

TEST_F(GbJoypadTest, ProcessKeyEvent)
{
    gbJoypad->write(0xFF00, 0b1110'0000);
    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(4);

    keyPress(Qt::Key_S);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b0111);
    keyRelease(Qt::Key_S);

    keyPress(Qt::Key_W);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b1011);
    keyRelease(Qt::Key_W);

    keyPress(Qt::Key_A);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b1101);
    keyRelease(Qt::Key_A);

    keyPress(Qt::Key_D);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b1110);
    keyRelease(Qt::Key_D);


    gbJoypad->write(0xFF00, 0b1101'0000);
    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(4);

    keyPress(Qt::Key_3);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b0111);
    keyRelease(Qt::Key_3);

    keyPress(Qt::Key_2);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b1011);
    keyRelease(Qt::Key_2);

    keyPress(Qt::Key_K);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b1101);
    keyRelease(Qt::Key_K);

    keyPress(Qt::Key_J);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0b1110);
    keyRelease(Qt::Key_J);
}

TEST_F(GbJoypadTest, Reset)
{
    gbJoypad->reset();
    EXPECT_EQ(gbJoypad->read(0xFF00), 0xCF);
}

}  // namespace qnemuTest
