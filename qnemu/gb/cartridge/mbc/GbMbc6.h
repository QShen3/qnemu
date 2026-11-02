/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"
#include "qnemu/gb/const.h"

namespace qnemu
{

/**
 * MBC6: the mapper used only by Net de Get: Minigame @ 100.
 *
 * The cartridge has a fixed 16 KiB ROM bank, two independent 8 KiB
 * ROM/flash windows and two independent 4 KiB RAM windows.  A 1 MiB
 * Macronix MX29F008 flash chip can be mapped into either window; the game
 * uses it to store downloaded minigames.
 *
 * Flash commands are issued through the same windows.  The implementation
 * models the 128-byte program buffer, the 128 KiB sectors, status/ID modes
 * and the sector 0 write protection closely enough for Net de Get's
 * erase/program/verify loop.
 *
 * The flash array is currently in-memory only.  Persisting it and emulating
 * the Mobile Adapter GB download path are separate follow-ups.
 */
class GbMbc6 : public GbMbcInterface
{
public:
    static constexpr uint16_t ramEnableAddress = 0x0000;
    static constexpr uint16_t ramBankAAddress = 0x0400;
    static constexpr uint16_t ramBankBAddress = 0x0800;
    static constexpr uint16_t flashEnableAddress = 0x0C00;
    static constexpr uint16_t flashWriteEnableAddress = 0x1000;
    static constexpr uint16_t romBankAAddress = 0x2000;
    static constexpr uint16_t romBankASelectAddress = 0x2800;
    static constexpr uint16_t romBankBAddress = 0x3000;
    static constexpr uint16_t romBankBSelectAddress = 0x3800;
    static constexpr uint16_t externalRamStart = 0xA000;
    static constexpr uint16_t externalRamEnd = 0xBFFF;

    static constexpr uint32_t flashSize = 0x1'00000;
    static constexpr uint32_t flashSectorSize = 0x2'0000;
    static constexpr std::size_t flashPageSize = 0x80;
    static constexpr std::size_t hiddenFlashSize = 0x100;

    GbMbc6() = delete;
    GbMbc6(std::vector<std::array<uint8_t, RomBankSize>>&& romBanks,
        std::vector<std::array<uint8_t, RamBankSize>>&& ramBanks, uint8_t type);
    ~GbMbc6() override = default;

    uint8_t read(uint16_t address) const override;
    void write(uint16_t address, const uint8_t value) override;
    void reset() override;

    bool hasBattery() const override;
    std::size_t ramSize() const override;
    std::vector<uint8_t> dumpRam() const override;
    void loadRam(const std::vector<uint8_t>& data) override;

private:
    enum class FlashPhase
    {
        idle,
        unlock1,
        unlock2,
        unlock3,
        unlock4,
        unlock5,
        program,
        programHidden,
    };

    uint8_t readRomWindow(uint16_t address, uint8_t bank) const;
    uint8_t readRamWindow(uint16_t address, uint8_t bank) const;
    void writeRamWindow(uint16_t address, uint8_t bank, uint8_t value);

    uint32_t flashAddress(uint16_t address) const;
    uint8_t readFlash(uint32_t address) const;
    void writeFlash(uint32_t address, uint8_t value);
    void handleFlashCommand(uint32_t address, uint8_t value);
    void startProgram();
    void writeProgramBuffer(uint32_t address, uint8_t value, bool hidden);
    void programPage(uint32_t address, bool hidden);
    void eraseSector(uint32_t address);
    void eraseChip();
    void eraseHidden();
    void resetFlashCommand();

    std::vector<std::array<uint8_t, RomBankSize>> romBanks;
    std::vector<std::array<uint8_t, RamBankSize>> ramBanks;
    uint8_t type;

    bool ramEnabled = false;
    uint8_t ramBankA = 0;
    uint8_t ramBankB = 0;
    bool flashEnabled = false;
    bool flashWriteEnabled = false;
    bool flashSelectedA = false;
    bool flashSelectedB = false;
    // Power-up default: window A = first half of ROM bank 1, window B = second
    // half, so 0x4000-0x7FFF looks like a normal 16 KiB bank 1.
    uint8_t romBankA = 2;
    uint8_t romBankB = 3;

    std::vector<uint8_t> flash;
    std::array<uint8_t, hiddenFlashSize> hiddenFlash {};

    FlashPhase flashPhase = FlashPhase::idle;
    uint8_t flashPendingCommand = 0;
    bool flashIdMode = false;
    bool flashStatusMode = false;
    bool flashHiddenRead = false;
    bool sector0Protected = false;

    std::array<uint8_t, flashPageSize> programBuffer {};
    uint8_t lastProgramOffset = 0;
    bool programBufferValid = false;
};

}  // namespace qnemu
