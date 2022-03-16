/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <gmock/gmock.h>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemuMock
{

class MockGbMbc : public qnemu::GbMbcInterface
{
public:
    MOCK_METHOD(uint8_t, read, (uint16_t), (const, override));
    MOCK_METHOD(void, write, (uint16_t, (const uint8_t&)), (override));
    MOCK_METHOD(void, reset, (), (override));
    MOCK_METHOD(void, step, (), (override));
    MOCK_METHOD(bool, accepts, (uint16_t), (const, override));
};

}  // namespace qnemuMock