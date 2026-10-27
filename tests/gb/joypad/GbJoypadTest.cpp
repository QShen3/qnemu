/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include <gtest/gtest.h>

#include "mock/gb/interrupt/MockGbInterruptHandler.h"
#include "qnemu/gb/joypad/GbJoypad.h"

namespace qnemuTest
{

class GbJoypadTest : public testing::Test
{
public:
    void SetUp() override
    {
        mockInterruptHandler =
            std::make_shared<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>>();
        gbJoypad = std::make_unique<qnemu::GbJoypad>(mockInterruptHandler);
    }

// NOLINTBEGIN
protected:
    std::shared_ptr<testing::StrictMock<qnemuMock::MockGbInterruptHanlder>> mockInterruptHandler;
    std::unique_ptr<qnemu::GbJoypad> gbJoypad;
// NOLINTEND
};

TEST_F(GbJoypadTest, ResetAndReadMasks)
{
    EXPECT_EQ(gbJoypad->read(0xFF00), 0xCF);

    // Neither group selected: low nibble is released, bits 6-7 read 1.
    gbJoypad->write(0xFF00, 0xF0);
    EXPECT_EQ(gbJoypad->read(0xFF00), 0xFF);

    // Bits 6-7 are ignored on writes; both groups selected reads 0xCF.
    gbJoypad->write(0xFF00, 0x0F);
    EXPECT_EQ(gbJoypad->read(0xFF00), 0xCF);

    // Lower nibble writes are ignored too.
    gbJoypad->write(0xFF00, 0x3F);
    EXPECT_EQ(gbJoypad->read(0xFF00), 0xFF);
}

TEST_F(GbJoypadTest, DirectionButtons)
{
    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(testing::AnyNumber());
    gbJoypad->write(0xFF00, 0xE0);  // select d-pad

    gbJoypad->setButtonState(qnemu::GbButton::right, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0E);
    gbJoypad->setButtonState(qnemu::GbButton::right, false);

    gbJoypad->setButtonState(qnemu::GbButton::left, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0D);
    gbJoypad->setButtonState(qnemu::GbButton::left, false);

    gbJoypad->setButtonState(qnemu::GbButton::up, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0B);
    gbJoypad->setButtonState(qnemu::GbButton::up, false);

    gbJoypad->setButtonState(qnemu::GbButton::down, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x07);
    gbJoypad->setButtonState(qnemu::GbButton::down, false);

    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0F);
}

TEST_F(GbJoypadTest, ActionButtons)
{
    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(testing::AnyNumber());
    gbJoypad->write(0xFF00, 0xD0);  // select action buttons

    gbJoypad->setButtonState(qnemu::GbButton::a, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0E);
    gbJoypad->setButtonState(qnemu::GbButton::a, false);

    gbJoypad->setButtonState(qnemu::GbButton::b, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0D);
    gbJoypad->setButtonState(qnemu::GbButton::b, false);

    gbJoypad->setButtonState(qnemu::GbButton::select, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0B);
    gbJoypad->setButtonState(qnemu::GbButton::select, false);

    gbJoypad->setButtonState(qnemu::GbButton::start, true);
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x07);
    gbJoypad->setButtonState(qnemu::GbButton::start, false);

    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0F);
}

TEST_F(GbJoypadTest, BothGroupsAreAnded)
{
    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(2);
    gbJoypad->write(0xFF00, 0xC0);  // select both groups

    gbJoypad->setButtonState(qnemu::GbButton::right, true);  // P10 low
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0E);

    gbJoypad->setButtonState(qnemu::GbButton::a, true);  // same P10 line
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0E);

    gbJoypad->setButtonState(qnemu::GbButton::b, true);  // P11 low
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0C);

    gbJoypad->setButtonState(qnemu::GbButton::right, false);  // A still holds P10
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0C);
}

TEST_F(GbJoypadTest, SelectionChangeRequestsInterruptForHeldButton)
{
    gbJoypad->write(0xFF00, 0xF0);  // neither group selected
    gbJoypad->setButtonState(qnemu::GbButton::right, true);  // not selected yet

    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(1);
    gbJoypad->write(0xFF00, 0xE0);  // selecting the d-pad creates an edge
    EXPECT_EQ(gbJoypad->read(0xFF00) & 0x0F, 0x0E);
}

TEST_F(GbJoypadTest, InterruptsAreEdgeTriggered)
{
    gbJoypad->write(0xFF00, 0xE0);  // select d-pad

    EXPECT_CALL(*mockInterruptHandler, requestJoypadInterrupt()).Times(4);

    gbJoypad->setButtonState(qnemu::GbButton::right, true);  // P10 high->low
    gbJoypad->setButtonState(qnemu::GbButton::right, true);  // key repeat: no edge
    gbJoypad->setButtonState(qnemu::GbButton::left, true);   // P11 high->low
    gbJoypad->setButtonState(qnemu::GbButton::right, false); // release: no edge
    gbJoypad->setButtonState(qnemu::GbButton::right, true);  // P10 high->low again
    gbJoypad->setButtonState(qnemu::GbButton::a, true);      // actions not selected
    gbJoypad->write(0xFF00, 0xD0);  // selecting actions while A is held
    gbJoypad->setButtonState(qnemu::GbButton::b, true);      // P11 high->low
    gbJoypad->setButtonState(qnemu::GbButton::a, false);     // release: no edge
}

TEST_F(GbJoypadTest, ReadWriteError)
{
#ifdef NDEBUG
    EXPECT_EQ(gbJoypad->read(0xFF01), 0xFF);
    EXPECT_NO_THROW(gbJoypad->write(0xFF01, 0xFF));
#else
    EXPECT_DEATH(gbJoypad->read(0xFF01), "");
    EXPECT_DEATH(gbJoypad->write(0xFF01, 0), "");
#endif
}

TEST_F(GbJoypadTest, Step)
{
    EXPECT_NO_THROW(gbJoypad->step());
}

}  // namespace qnemuTest
