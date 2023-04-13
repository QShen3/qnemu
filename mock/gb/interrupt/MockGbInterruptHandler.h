/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <functional>

#include <gmock/gmock.h>

#include "qnemu/gb/interrupt/GbInterruptHandlerInterface.h"

namespace qnemuMock
{

class MockGbInterruptHanlder : public qnemu::GbInterruptHandlerInterface
{
public:
    MOCK_METHOD(uint8_t, read, (uint16_t), (const, override));
    MOCK_METHOD(void, write, (uint16_t, (const uint8_t&)), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, step, (), (override));

    MOCK_METHOD(void, registerCpuCallback, (std::function<void(qnemu::GbInterrupt)>), (override));
    MOCK_METHOD(void, requestVBlankInterrupt, (), (override));
    MOCK_METHOD(void, requestLcdInterrupt, (), (override));
    MOCK_METHOD(void, requestTimerInterrupt, (), (override));
    MOCK_METHOD(void, requestSerialInterrupt, (), (override));
    MOCK_METHOD(void, requestJoypadInterrupt, (), (override));
};

}  // namespace qnemuMock
