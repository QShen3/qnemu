/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <gmock/gmock.h>

#include "qnemu/gb/gpu/GbGpuInterface.h"

namespace qnemuMock
{

class MockGbGpu : public qnemu::GbGpuInterface
{
public:
    MOCK_METHOD(uint8_t, read, (uint16_t), (const, override));
    MOCK_METHOD(void, write, (uint16_t, (const uint8_t&)), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, step, (), (override));
    MOCK_METHOD(bool, accepts, (uint16_t), (const, override));

    MOCK_METHOD(uint8_t, currentMode, (), (const, override));
    MOCK_METHOD(bool, isLcdEnable, (), (const, override));
};

}  // namespace qnemuMock
