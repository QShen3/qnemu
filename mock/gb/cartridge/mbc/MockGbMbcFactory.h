/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <gmock/gmock.h>

#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemuMock
{

class MockGbMbcFactory : public qnemu::GbMbcFactoryInterface
{
public:
    MOCK_METHOD(
        (std::unique_ptr<qnemu::GbMbcInterface>),
        create,
        ((std::vector<std::array<uint8_t, qnemu::RomBankSize>>&&),
            (std::vector<std::array<uint8_t, qnemu::RamBankSize>>&&), uint8_t),
        (const, override)
    );
};

}  // namespace qnemuMock