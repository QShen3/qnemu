/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <gmock/gmock.h>

#include "qnemu/gb/interrupt/GbInterruptHanlerInterface.h"

namespace qnemuMock
{

class MockGbInterruptHanlder : public qnemu::GbInterruptHandlerInterface
{
public:
    MOCK_METHOD(void, requestVBlankInterrupt, (), (override));
    MOCK_METHOD(void, requestLcdInterrupt, (), (override));
    MOCK_METHOD(void, requestTimerInterrupt, (), (override));
    MOCK_METHOD(void, requestSerialInterrupt, (), (override));
    MOCK_METHOD(void, requestJoypadInterrupt, (), (override));
};

}  // namespace qnemuMock
