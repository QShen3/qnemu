/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <gmock/gmock.h>

#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemuMock
{

class MockGbMbcFactory : public qnemu::GbMbcFactoryInterface
{
public:
    MOCK_METHOD(
        (std::unique_ptr<qnemu::GbMbcInterface>),
        create,
        ((std::vector<std::array<uint8_t, qnemu::GbMbcInterface::romBankSize>>&&),
            (std::vector<std::array<uint8_t, qnemu::GbMbcInterface::ramBankSize>>&&), uint8_t),
        (const, override)
    );
};

}  // namespace qnemuMock