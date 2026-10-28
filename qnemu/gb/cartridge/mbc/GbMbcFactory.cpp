/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbCircuitMbc.h"
#include "qnemu/gb/cartridge/mbc/GbMbc1.h"
#include "qnemu/gb/cartridge/mbc/GbMbc1M.h"
#include "qnemu/gb/cartridge/mbc/GbMbc2.h"
#include "qnemu/gb/cartridge/mbc/GbMbc3.h"
#include "qnemu/gb/cartridge/mbc/GbMbc5.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactory.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

namespace
{

bool isMbc1Multicart(const std::vector<std::array<uint8_t, RomBankSize>>& romBanks)
{
    // MBC1M carts are 1 MiB and normally contain another cartridge header
    // (including the Nintendo logo) in bank $10.  Regular 1 MiB MBC1 carts
    // do not.  This is the same heuristic used by other emulators.
    if (romBanks.size() != 64) {
        return false;
    }

    static constexpr uint8_t logoPrefix[] = {
        0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    };
    const auto& bank = romBanks.at(0x10);
    for (std::size_t i = 0; i < sizeof(logoPrefix); ++i) {
        if (bank.at(0x104 + i) != logoPrefix[i]) {
            return false;
        }
    }
    return true;
}

}  // namespace

std::unique_ptr<GbMbcInterface> GbMbcFactory::create(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type) const
{
    std::unique_ptr<GbMbcInterface> mbc;

    if (type == 0x0 || type == 0x8 || type == 0x9) {
        mbc = std::make_unique<GbCircuitMbc>(std::move(romBanks), std::move(ramBanks), type);
    } else if (type == 0x1 || type == 0x2 || type == 0x3) {
        if (isMbc1Multicart(romBanks)) {
            mbc = std::make_unique<GbMbc1M>(
                std::move(romBanks), std::move(ramBanks), type);
        } else {
            mbc = std::make_unique<GbMbc1>(
                std::move(romBanks), std::move(ramBanks), type);
        }
    } else if (type == 0x5 || type == 0x6) {
        mbc = std::make_unique<GbMbc2>(std::move(romBanks), std::move(ramBanks), type);
    } else if (type >= 0x0F && type <= 0x13) {
        mbc = std::make_unique<GbMbc3>(std::move(romBanks), std::move(ramBanks), type);
    } else if (type >= 0x19 && type <= 0x1E) {
        mbc = std::make_unique<GbMbc5>(std::move(romBanks), std::move(ramBanks), type);
    } else {
        throw std::runtime_error("This game is not supported!");
    }

    return mbc;
}

}  // namespace qnemu
