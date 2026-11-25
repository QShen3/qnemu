/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

#include "qnemu/gb/cartridge/mbc/GbAccelerometerState.h"
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

class GbMbcFactory : public GbMbcFactoryInterface
{
    public:
        explicit GbMbcFactory(std::shared_ptr<AccelerometerState> accelerometer = nullptr) :
            accelerometer(accelerometer ? std::move(accelerometer)
                                        : std::make_shared<AccelerometerState>())
        {
        }
        ~GbMbcFactory() override = default;

        std::unique_ptr<GbMbcInterface> create(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
            std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) const override;

    private:
        std::shared_ptr<AccelerometerState> accelerometer;
};

}  // namespace qnemu

