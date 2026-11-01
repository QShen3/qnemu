/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>
#include <memory>
#include <utility>

#include "qnemu/gb/const.h"
#include "qnemu/gb/mmu/DmgMmu.h"

namespace qnemu
{

DmgMmu::DmgMmu(std::unique_ptr<GbApu> apu,
        std::shared_ptr<GbCartridgeInterface> cartridge,
        std::unique_ptr<GbGpuInterface> gpu,
        std::unique_ptr<GbHighRam> highRam,
        std::shared_ptr<GbInterruptHandlerInterface> interruptHandler,
        std::unique_ptr<GbJoypad> joypad,
        std::unique_ptr<DmgWorkRam> workRam,
        std::unique_ptr<GbTimer> timer,
        std::unique_ptr<GbSerial> serial) :
    apu(std::move(apu)),
    cartridge(cartridge),
    gpu_(std::move(gpu)),
    highRam(std::move(highRam)),
    interruptHandler(interruptHandler),
    joypad(std::move(joypad)),
    workRam_(std::move(workRam)),
    timer_(std::move(timer)),
    serial_(std::move(serial))
{
}

uint8_t DmgMmu::read(uint16_t address) const
{
    if (bootRomActive && !bootRom.empty()
        && (address <= 0x00FF
            || (bootRom.size() >= 0x0900 && address >= 0x0200 && address <= 0x08FF))) {
        return bootRom[address];
    }
    if (address <= MemoryRomBank01End) {
        return cartridge->read(address);
    } else if (address <= VideoRamEnd) {
        return gpu_->read(address);
    } else if (address <= ExternalRamEnd) {
        return cartridge->read(address);
    } else if (address <= EchoRamEnd) {
        return workRam_->read(address);
    } else if (address <= 0xFEFF) {
        return gpu_->read(address);
    } else if (address == 0xFF00) {
        return joypad->read(address);
    } else if (address >= 0xFF01 && address <= 0xFF02) {
        return serial_->read(address);
    } else if (address >= 0xFF04 && address <= 0xFF07) {
        return timer_->read(address);
    } else if (address >= 0xFF10 && address <= 0xFF26) {
        return apu->read(address);
    } else if (address == 0xFF76 || address == 0xFF77) {
        return apu->read(address);
    } else if (address >= 0xFF30 && address <= 0xFF3F) {
        return apu->read(address);
    } else if (address >= 0xFF40 && address <= 0xFF4B) {
        return gpu_->read(address);
    } else if (address == 0xFF50) {
        return 0xFF;
    } else if (address >= HighRamStart && address <= HighRamEnd) {
        return highRam->read(address);
    } else if (address == 0xFF0F || address == 0xFFFF) {
        return interruptHandler->read(address);
    } else {
        return 0xFF;
    }
}

void DmgMmu::write(uint16_t address, const uint8_t& value)
{
    if (address == 0xFF50) {
        bootRomActive = false;
        return;
    }
    if (address <= MemoryRomBank01End) {
        cartridge->write(address, value);
    } else if (address <= VideoRamEnd) {
        gpu_->write(address, value);
    } else if (address <= ExternalRamEnd) {
        cartridge->write(address, value);
    } else if (address <= EchoRamEnd) {
        workRam_->write(address, value);
    } else if (address <= OamEnd) {
        gpu_->write(address, value);
    } else if (address == 0xFF00) {
        joypad->write(address, value);
    } else if (address >= 0xFF01 && address <= 0xFF02) {
        serial_->write(address, value);
    } else if (address >= 0xFF04 && address <= 0xFF07) {
        timer_->write(address, value);
        if (timer_->takeDivApuTick()) {
            apu->divApuTick();
        }
    } else if (address >= 0xFF10 && address <= 0xFF26) {
        if (address == 0xFF26) {
            const bool wasPowered = apu->isPowered();
            apu->write(address, value);
            if (!wasPowered && apu->isPowered() && timer_->isDivApuBitHigh()) {
                apu->skipNextDivApuTick();
            }
        } else {
            apu->write(address, value);
        }
    } else if (address >= 0xFF30 && address <= 0xFF3F) {
        apu->write(address, value);
    } else if (address >= 0xFF40 && address <= 0xFF4B) {
        gpu_->write(address, value);
    } else if (address >= HighRamStart && address <= HighRamEnd) {
        highRam->write(address, value);
    } else if (address == 0xFF0F || address == 0xFFFF) {
        interruptHandler->write(address, value);
    }
}

void DmgMmu::step()
{
    apu->step();
    cartridge->step();
    gpu_->step();
    workRam_->step();
    highRam->step();
    timer_->step();
    if (timer_->takeDivApuTick()) {
        apu->divApuTick();
    }
    joypad->step();
    interruptHandler->step();
    serial_->step();
}

void DmgMmu::reset()
{
    apu->reset();
    cartridge->reset();
    gpu_->reset();
    workRam_->reset();
    highRam->reset();
    timer_->reset();
    joypad->reset();
    interruptHandler->reset();
    serial_->reset();
}

bool DmgMmu::isCpuStalled() const
{
    return gpu_->isHdmaActive();
}

void DmgMmu::triggerOamBugWrite(uint16_t address, int rowOffset)
{
    gpu_->triggerOamBugWrite(address, rowOffset);
}

void DmgMmu::triggerOamBugRead(uint16_t address)
{
    gpu_->triggerOamBugRead(address);
}

void DmgMmu::triggerOamBugReadIncrease(uint16_t address)
{
    gpu_->triggerOamBugReadIncrease(address);
}

}  // namespace qnemu
