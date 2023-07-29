/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <gmock/gmock.h>

#include "qnemu/gb/GbDeviceInterface.h"

namespace qnemuMock
{

class MockGbDevice : public qnemu::GbDeviceInterface
{
public:
    MOCK_METHOD(uint8_t, read, (uint16_t), (const, override));
    MOCK_METHOD(void, write, (uint16_t, (const uint8_t&)), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, step, (), (override));
};

}  // namespace qnemuMock
