/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <gmock/gmock.h>

#include "qnemu/gb/cartridge/GbCartridgeInterface.h"

namespace qnemuMock
{

class MockGbCartridge : public qnemu::GbCartridgeInterface
{
public:
    MOCK_METHOD(uint8_t, read, (uint16_t), (const, override));
    MOCK_METHOD(void, write, (uint16_t, (const uint8_t&)), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, step, (), (override));

    MOCK_METHOD(void, load, (const char*), (override));
    MOCK_METHOD(bool, isGbcCartridge, (), (const, override));
};

}  // namespace qnemuMock