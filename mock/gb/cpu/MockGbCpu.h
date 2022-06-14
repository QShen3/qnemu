/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>
#include <memory>

#include <gmock/gmock.h>

#include "qnemu/gb/GbDeviceInterface.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"

namespace qnemuMock
{

class MockGbCpu : public qnemu::GbCpuInterface
{
public:
    MOCK_METHOD(void, start, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, reset, (), (override));

    MOCK_METHOD(void, jumpToAddress, (uint16_t), (override));
    MOCK_METHOD(void, cancelInterrupt, (), (override));
    MOCK_METHOD(void, addDevice, (std::shared_ptr<qnemu::GbDeviceInterface>), (override));

    MOCK_METHOD(uint8_t, readByte, (uint16_t), (const, override));
    MOCK_METHOD(void, writeByte, (uint16_t, uint8_t), (override));
};

}  // namespace qnemuMock