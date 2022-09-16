/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>

#include "qnemu/gb/const.h"
#include "qnemu/gb/cpu/GbCpuInterface.h"
#include "qnemu/gb/gpu/SpriteAttributeTable.h"

namespace qnemu
{

SpriteAttributeTable::SpriteAttributeTable(std::shared_ptr<GbCpuInterface> cpu) : cpu(cpu)
{
    SpriteAttributeTable::reset();
}

bool SpriteAttributeTable::accepts(uint16_t address) const
{
    if (address >= SpriteAttributeTableStart && address <= SpriteAttributeTableEnd) {
        return true;
    }
    if (address == 0xFF46) {
        return true;
    }
    return false;
}

uint8_t SpriteAttributeTable::read(uint16_t address) const
{
    if (address >= SpriteAttributeTableStart && address <= SpriteAttributeTableEnd) {
        return data.at(address - SpriteAttributeTableStart);
    }
    if (address == 0xFF46) {
        return registers.dmaTransferAndStartAddress;
    }
    assert(false && "Wrong address");
    return 0xFF;
}

void SpriteAttributeTable::write(uint16_t address, const uint8_t& value)
{
    if (address >= SpriteAttributeTableStart && address <= SpriteAttributeTableEnd) {
        data.at(address - SpriteAttributeTableStart) = value;
    }
    if (address == 0xFF46) {
        registers.dmaTransferAndStartAddress = value;
        if (!isDmaInProgress) {
            isDmaInProgress = true;
            dmaTicks = 640;
        }
    }
}

void SpriteAttributeTable::step()
{
    if (!isDmaInProgress) {
        return;
    }

    dmaTicks--;

    if (dmaTicks > 0) {
        return;
    }
    isDmaInProgress = false;
    for (uint8_t i = 0; i < 0xA0; i++) {
        data.at(i) = cpu.lock()->readByte(registers.dmaTransferAndStartAddress * 0x100 + i);
    }
}

void SpriteAttributeTable::reset()
{
    std::memset(&registers, 0, sizeof(registers));

    isDmaInProgress = false;
    dmaTicks = 0;
}

uint8_t SpriteAttributeTable::at(uint16_t address) const
{
    return data.at(address);
}

}  // namespace qnemu