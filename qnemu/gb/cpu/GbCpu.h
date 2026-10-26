/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <cstdint>

#include "qnemu/gb/cpu/CpuBus.h"
#include "qnemu/gb/cpu/Registers.h"

namespace qnemu
{

/**
 * SM83 (Game Boy / Game Boy Color CPU) core.
 *
 * The core is deterministic and single-threaded: step() executes exactly one
 * instruction (or one interrupt dispatch/idle machine cycle) and advances the
 * rest of the machine through CpuBus::tick().  It owns no MMU, no display and
 * no thread, which makes it usable from unit tests and from the headless ROM
 * runner as well as from the GUI.
 */
class GbCpu
{
public:
    explicit GbCpu(CpuBus& bus);
    ~GbCpu() = default;

    GbCpu(const GbCpu&) = delete;
    GbCpu& operator=(const GbCpu&) = delete;

    void reset();

    /**
     * Execute one instruction, service a pending interrupt, or idle one
     * machine cycle while HALT/STOP is active.
     */
    void step();

    const Registers& registers() const { return registers_; }
    Registers& registers() { return registers_; }

    uint64_t cycles() const { return cycles_; }
    bool isHalted() const { return halted_; }
    bool isStopped() const { return stopped_; }
    bool interruptsEnabled() const { return ime_; }
    uint64_t invalidOpcodeCount() const { return invalidOpcodeCount_; }

private:
    // Bus helpers.  Every read/write is one M-cycle.
    uint8_t read8(uint16_t address);
    void write8(uint16_t address, uint8_t value);
    uint8_t fetch8();
    uint16_t fetch16();
    void tick(uint8_t cycles);

    // Register-file helpers used by the regular opcode blocks.
    uint8_t readReg(uint8_t index);
    void writeReg(uint8_t index, uint8_t value);
    uint16_t readReg16(uint8_t index);
    void writeReg16(uint8_t index, uint16_t value);

    // Control flow helpers.
    uint16_t pop16();
    void push16(uint16_t value);
    void ret();
    void retConditional(uint8_t condition);
    void jrConditional(uint8_t condition);
    void jpConditional(uint8_t condition);
    void callConditional(uint8_t condition);
    void rst(uint16_t vector);
    bool condition(uint8_t index) const;

    // Interrupts and sleep states.
    uint8_t pendingInterrupts();
    void dispatchInterrupt(uint8_t pending);
    void halt();
    void stop();

    // Instruction groups.
    void execute(uint8_t opcode);
    void executeCb(uint8_t opcode);
    void executeAlu(uint8_t operation, uint8_t value);
    void executeLoad(uint8_t destination, uint8_t source);
    void invalidOpcode();

    // ALU helpers.  They update F exactly once, which keeps the flag rules in
    // one place instead of spreading them across 512 instruction functions.
    void add(uint8_t value, bool withCarry);
    void subtract(uint8_t value, bool withCarry);
    void compare(uint8_t value);
    void andWith(uint8_t value);
    void xorWith(uint8_t value);
    void orWith(uint8_t value);
    void addHl(uint16_t value);
    void addSp(int8_t offset);
    void loadHlFromSpOffset(int8_t offset);
    uint8_t increment(uint8_t value);
    uint8_t decrement(uint8_t value);
    void daa();
    void cpl();
    void scf();
    void ccf();
    void rlca();
    void rrca();
    void rla();
    void rra();

    // CB-prefixed operations.
    void rotateLeft(uint8_t index);
    void rotateRight(uint8_t index);
    void rotateLeftThroughCarry(uint8_t index);
    void rotateRightThroughCarry(uint8_t index);
    void shiftLeftArithmetic(uint8_t index);
    void shiftRightArithmetic(uint8_t index);
    void shiftRightLogical(uint8_t index);
    void swap(uint8_t index);
    void bit(uint8_t bitIndex, uint8_t index);
    void res(uint8_t bitIndex, uint8_t index);
    void set(uint8_t bitIndex, uint8_t index);

    // Flag helpers.
    bool flag(uint8_t flag) const { return (registers_.f & flag) != 0; }
    void setFlag(uint8_t flag, bool value)
    {
        if (value) {
            registers_.f |= flag;
        } else {
            registers_.f &= static_cast<uint8_t>(~flag);
        }
    }
    void setZeroFlag(uint8_t value) { setFlag(CpuFlags::zero, value == 0); }
    void setHalfCarryFlag(bool value) { setFlag(CpuFlags::halfCarry, value); }
    void setCarryFlag(bool value) { setFlag(CpuFlags::carry, value); }
    void setSubtractFlag(bool value) { setFlag(CpuFlags::subtract, value); }

    CpuBus& bus;
    Registers registers_;

    bool ime_ = false;
    uint8_t imeDelay_ = 0;
    bool halted_ = false;
    bool stopped_ = false;
    bool haltBug_ = false;

    uint64_t cycles_ = 0;
    uint64_t invalidOpcodeCount_ = 0;
};

}  // namespace qnemu
