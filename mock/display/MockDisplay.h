/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <gmock/gmock.h>

#include "qnemu/display/DisplayInterface.h"

namespace qnemuMock
{

class MockDisplay : public qnemu::DisplayInterface
{
public:
    MOCK_METHOD(void, presentFrame, (const qnemu::FrameBuffer&), (override));
    MOCK_METHOD(void, enable, (), (override));
    MOCK_METHOD(void, disable, (), (override));
};

}  // namespace qnemuMock
