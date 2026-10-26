/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/cpu/GbCpu.h"

namespace qnemu
{

namespace
{
constexpr uint16_t interruptEnableAddress = 0xFFFF;
constexpr uint16_t interruptFlagAddress = 0xFF0F;
constexpr uint8_t joypadInterruptBit = 0x10;
}  // namespace

GbCpu::GbCpu(CpuBus& bus) : bus(bus)
{
    reset();
}

void GbCpu::reset()
{
    // Post-boot CGB register values.  The emulator has no boot ROM, so the
    // first fetched instruction is the cartridge entry point at 0x0100.
    registers_ = Registers{};
    ime_ = false;
    imeDelay_ = 0;
    halted_ = false;
    stopped_ = false;
    haltBug_ = false;
    cycles_ = 0;
    invalidOpcodeCount_ = 0;
}

void GbCpu::step()
{
    const uint8_t pending = pendingInterrupts();

    if (ime_ && pending != 0) {
        dispatchInterrupt(pending);
        return;
    }

    if (halted_) {
        if (pending != 0) {
            halted_ = false;
        } else {
            tick(4);
            return;
        }
    }

    if (stopped_) {
        // STOP is left by a joypad line going low; the joypad request bit is
        // the closest observable equivalent in this core.
        if ((bus.read(interruptFlagAddress) & joypadInterruptBit) != 0) {
            stopped_ = false;
        } else {
            tick(4);
            return;
        }
    }

    const uint8_t opcode = fetch8();
    execute(opcode);

    // EI takes effect after the instruction following it.  Decrementing at
    // the end of each step gives exactly that one-instruction delay.
    if (imeDelay_ > 0) {
        --imeDelay_;
        if (imeDelay_ == 0) {
            ime_ = true;
        }
    }
}

uint8_t GbCpu::read8(uint16_t address)
{
    const uint8_t value = bus.read(address);
    tick(4);
    return value;
}

void GbCpu::write8(uint16_t address, uint8_t value)
{
    bus.write(address, value);
    tick(4);
}

uint8_t GbCpu::fetch8()
{
    const uint8_t value = bus.read(registers_.pc);
    tick(4);
    if (haltBug_) {
        // The HALT bug makes the byte after HALT execute twice: the first
        // fetch reads it without advancing PC.
        haltBug_ = false;
    } else {
        ++registers_.pc;
    }
    return value;
}

uint16_t GbCpu::fetch16()
{
    const uint8_t low = fetch8();
    const uint8_t high = fetch8();
    return static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8);
}

void GbCpu::tick(uint8_t cycles)
{
    bus.tick(cycles);
    cycles_ += cycles;
}

uint8_t GbCpu::readReg(uint8_t index)
{
    switch (index) {
        case 0: return registers_.b;
        case 1: return registers_.c;
        case 2: return registers_.d;
        case 3: return registers_.e;
        case 4: return registers_.h;
        case 5: return registers_.l;
        case 6: return read8(registers_.hl());
        default: return registers_.a;
    }
}

void GbCpu::writeReg(uint8_t index, uint8_t value)
{
    switch (index) {
        case 0: registers_.b = value; break;
        case 1: registers_.c = value; break;
        case 2: registers_.d = value; break;
        case 3: registers_.e = value; break;
        case 4: registers_.h = value; break;
        case 5: registers_.l = value; break;
        case 6: write8(registers_.hl(), value); break;
        default: registers_.a = value; break;
    }
}

uint16_t GbCpu::readReg16(uint8_t index)
{
    switch (index) {
        case 0: return registers_.bc();
        case 1: return registers_.de();
        case 2: return registers_.hl();
        default: return registers_.sp;
    }
}

void GbCpu::writeReg16(uint8_t index, uint16_t value)
{
    switch (index) {
        case 0: registers_.setBc(value); break;
        case 1: registers_.setDe(value); break;
        case 2: registers_.setHl(value); break;
        default: registers_.sp = value; break;
    }
}

uint16_t GbCpu::pop16()
{
    const uint8_t low = read8(registers_.sp);
    ++registers_.sp;
    const uint8_t high = read8(registers_.sp);
    ++registers_.sp;
    return static_cast<uint16_t>(low) | (static_cast<uint16_t>(high) << 8);
}

void GbCpu::push16(uint16_t value)
{
    write8(static_cast<uint16_t>(registers_.sp - 1), static_cast<uint8_t>(value >> 8));
    write8(static_cast<uint16_t>(registers_.sp - 2), static_cast<uint8_t>(value & 0xFF));
    registers_.sp -= 2;
}

void GbCpu::ret()
{
    registers_.pc = pop16();
    tick(4);
}

void GbCpu::retConditional(uint8_t conditionIndex)
{
    // Not taken: 8 T-cycles.  Taken: 20 T-cycles (condition check, pop, and
    // the internal cycle after loading PC).
    tick(4);
    if (condition(conditionIndex)) {
        ret();
    }
}

void GbCpu::jrConditional(uint8_t conditionIndex)
{
    const int8_t offset = static_cast<int8_t>(fetch8());
    if (condition(conditionIndex)) {
        registers_.pc = static_cast<uint16_t>(registers_.pc + offset);
        tick(4);
    }
}

void GbCpu::jpConditional(uint8_t conditionIndex)
{
    const uint16_t address = fetch16();
    if (condition(conditionIndex)) {
        registers_.pc = address;
        tick(4);
    }
}

void GbCpu::callConditional(uint8_t conditionIndex)
{
    const uint16_t address = fetch16();
    if (condition(conditionIndex)) {
        tick(4);
        push16(registers_.pc);
        registers_.pc = address;
    }
}

void GbCpu::rst(uint16_t vector)
{
    tick(4);
    push16(registers_.pc);
    registers_.pc = vector;
}

bool GbCpu::condition(uint8_t index) const
{
    switch (index) {
        case 0: return !flag(CpuFlags::zero);
        case 1: return flag(CpuFlags::zero);
        case 2: return !flag(CpuFlags::carry);
        default: return flag(CpuFlags::carry);
    }
}

uint8_t GbCpu::pendingInterrupts()
{
    return static_cast<uint8_t>(bus.read(interruptFlagAddress)
        & bus.read(interruptEnableAddress)
        & 0x1F);
}

void GbCpu::dispatchInterrupt(uint8_t pending)
{
    uint8_t mask = 0x01;
    uint16_t vector = 0x40;
    for (uint8_t index = 0; index < 5; ++index) {
        if ((pending & mask) != 0) {
            vector = static_cast<uint16_t>(0x40 + index * 8);
            break;
        }
        mask = static_cast<uint8_t>(mask << 1);
    }

    bus.write(interruptFlagAddress,
        static_cast<uint8_t>(bus.read(interruptFlagAddress) & static_cast<uint8_t>(~mask)));
    ime_ = false;
    imeDelay_ = 0;
    halted_ = false;
    stopped_ = false;

    // Hardware takes five M-cycles: two internal, push high, push low, then
    // the final internal cycle before the vector is fetched.
    tick(8);
    write8(static_cast<uint16_t>(registers_.sp - 1), static_cast<uint8_t>(registers_.pc >> 8));
    write8(static_cast<uint16_t>(registers_.sp - 2), static_cast<uint8_t>(registers_.pc & 0xFF));
    registers_.sp -= 2;
    registers_.pc = vector;
    tick(4);
}

void GbCpu::halt()
{
    if (!ime_ && pendingInterrupts() != 0) {
        // HALT bug: the instruction after HALT is executed twice.
        haltBug_ = true;
    } else {
        halted_ = true;
    }
}

void GbCpu::stop()
{
    // STOP is encoded as 0x10 followed by a padding byte.
    fetch8();
    if (!bus.onStop()) {
        stopped_ = false;
        return;
    }
    stopped_ = true;
    if ((bus.read(interruptFlagAddress) & joypadInterruptBit) != 0) {
        stopped_ = false;
    }
}

void GbCpu::execute(uint8_t opcode)
{
    // 0x40-0x7F is the regular LD r,r' block (0x76 is HALT).
    if (opcode >= 0x40 && opcode <= 0x7F) {
        if (opcode == 0x76) {
            halt();
            return;
        }
        executeLoad(static_cast<uint8_t>((opcode >> 3) & 0x07), static_cast<uint8_t>(opcode & 0x07));
        return;
    }

    // 0x80-0xBF is the ALU A,r block.
    if (opcode >= 0x80 && opcode <= 0xBF) {
        executeAlu(static_cast<uint8_t>((opcode >> 3) & 0x07), readReg(static_cast<uint8_t>(opcode & 0x07)));
        return;
    }

    switch (opcode) {
        case 0x00:  // NOP
            break;
        case 0x01:  // LD BC, d16
            writeReg16(0, fetch16());
            break;
        case 0x02:  // LD (BC), A
            write8(registers_.bc(), registers_.a);
            break;
        case 0x03:  // INC BC
            registers_.setBc(static_cast<uint16_t>(registers_.bc() + 1));
            tick(4);
            break;
        case 0x04:  // INC B
            registers_.b = increment(registers_.b);
            break;
        case 0x05:  // DEC B
            registers_.b = decrement(registers_.b);
            break;
        case 0x06:  // LD B, d8
            registers_.b = fetch8();
            break;
        case 0x07:  // RLCA
            rlca();
            break;
        case 0x08: {  // LD (a16), SP
            const uint16_t address = fetch16();
            write8(address, static_cast<uint8_t>(registers_.sp & 0xFF));
            write8(static_cast<uint16_t>(address + 1), static_cast<uint8_t>(registers_.sp >> 8));
            break;
        }
        case 0x09:  // ADD HL, BC
            addHl(registers_.bc());
            break;
        case 0x0A:  // LD A, (BC)
            registers_.a = read8(registers_.bc());
            break;
        case 0x0B:  // DEC BC
            registers_.setBc(static_cast<uint16_t>(registers_.bc() - 1));
            tick(4);
            break;
        case 0x0C:  // INC C
            registers_.c = increment(registers_.c);
            break;
        case 0x0D:  // DEC C
            registers_.c = decrement(registers_.c);
            break;
        case 0x0E:  // LD C, d8
            registers_.c = fetch8();
            break;
        case 0x0F:  // RRCA
            rrca();
            break;

        case 0x10:  // STOP
            stop();
            break;
        case 0x11:  // LD DE, d16
            writeReg16(1, fetch16());
            break;
        case 0x12:  // LD (DE), A
            write8(registers_.de(), registers_.a);
            break;
        case 0x13:  // INC DE
            registers_.setDe(static_cast<uint16_t>(registers_.de() + 1));
            tick(4);
            break;
        case 0x14:  // INC D
            registers_.d = increment(registers_.d);
            break;
        case 0x15:  // DEC D
            registers_.d = decrement(registers_.d);
            break;
        case 0x16:  // LD D, d8
            registers_.d = fetch8();
            break;
        case 0x17:  // RLA
            rla();
            break;
        case 0x18: {  // JR e8
            const int8_t offset = static_cast<int8_t>(fetch8());
            registers_.pc = static_cast<uint16_t>(registers_.pc + offset);
            tick(4);
            break;
        }
        case 0x19:  // ADD HL, DE
            addHl(registers_.de());
            break;
        case 0x1A:  // LD A, (DE)
            registers_.a = read8(registers_.de());
            break;
        case 0x1B:  // DEC DE
            registers_.setDe(static_cast<uint16_t>(registers_.de() - 1));
            tick(4);
            break;
        case 0x1C:  // INC E
            registers_.e = increment(registers_.e);
            break;
        case 0x1D:  // DEC E
            registers_.e = decrement(registers_.e);
            break;
        case 0x1E:  // LD E, d8
            registers_.e = fetch8();
            break;
        case 0x1F:  // RRA
            rra();
            break;

        case 0x20:  // JR NZ, e8
            jrConditional(0);
            break;
        case 0x21:  // LD HL, d16
            writeReg16(2, fetch16());
            break;
        case 0x22: {  // LD (HL+), A
            const uint16_t address = registers_.hl();
            write8(address, registers_.a);
            registers_.setHl(static_cast<uint16_t>(address + 1));
            break;
        }
        case 0x23:  // INC HL
            registers_.setHl(static_cast<uint16_t>(registers_.hl() + 1));
            tick(4);
            break;
        case 0x24:  // INC H
            registers_.h = increment(registers_.h);
            break;
        case 0x25:  // DEC H
            registers_.h = decrement(registers_.h);
            break;
        case 0x26:  // LD H, d8
            registers_.h = fetch8();
            break;
        case 0x27:  // DAA
            daa();
            break;
        case 0x28:  // JR Z, e8
            jrConditional(1);
            break;
        case 0x29:  // ADD HL, HL
            addHl(registers_.hl());
            break;
        case 0x2A: {  // LD A, (HL+)
            const uint16_t address = registers_.hl();
            registers_.a = read8(address);
            registers_.setHl(static_cast<uint16_t>(address + 1));
            break;
        }
        case 0x2B:  // DEC HL
            registers_.setHl(static_cast<uint16_t>(registers_.hl() - 1));
            tick(4);
            break;
        case 0x2C:  // INC L
            registers_.l = increment(registers_.l);
            break;
        case 0x2D:  // DEC L
            registers_.l = decrement(registers_.l);
            break;
        case 0x2E:  // LD L, d8
            registers_.l = fetch8();
            break;
        case 0x2F:  // CPL
            cpl();
            break;

        case 0x30:  // JR NC, e8
            jrConditional(2);
            break;
        case 0x31:  // LD SP, d16
            writeReg16(3, fetch16());
            break;
        case 0x32: {  // LD (HL-), A
            const uint16_t address = registers_.hl();
            write8(address, registers_.a);
            registers_.setHl(static_cast<uint16_t>(address - 1));
            break;
        }
        case 0x33:  // INC SP
            registers_.sp += 1;
            tick(4);
            break;
        case 0x34: {  // INC (HL)
            const uint16_t address = registers_.hl();
            const uint8_t value = increment(read8(address));
            write8(address, value);
            break;
        }
        case 0x35: {  // DEC (HL)
            const uint16_t address = registers_.hl();
            const uint8_t value = decrement(read8(address));
            write8(address, value);
            break;
        }
        case 0x36: {  // LD (HL), d8
            const uint8_t value = fetch8();
            write8(registers_.hl(), value);
            break;
        }
        case 0x37:  // SCF
            scf();
            break;
        case 0x38:  // JR C, e8
            jrConditional(3);
            break;
        case 0x39:  // ADD HL, SP
            addHl(registers_.sp);
            break;
        case 0x3A: {  // LD A, (HL-)
            const uint16_t address = registers_.hl();
            registers_.a = read8(address);
            registers_.setHl(static_cast<uint16_t>(address - 1));
            break;
        }
        case 0x3B:  // DEC SP
            registers_.sp -= 1;
            tick(4);
            break;
        case 0x3C:  // INC A
            registers_.a = increment(registers_.a);
            break;
        case 0x3D:  // DEC A
            registers_.a = decrement(registers_.a);
            break;
        case 0x3E:  // LD A, d8
            registers_.a = fetch8();
            break;
        case 0x3F:  // CCF
            ccf();
            break;

        case 0xC0:  // RET NZ
            retConditional(0);
            break;
        case 0xC1:  // POP BC
            writeReg16(0, pop16());
            break;
        case 0xC2:  // JP NZ, a16
            jpConditional(0);
            break;
        case 0xC3:  // JP a16
            registers_.pc = fetch16();
            tick(4);
            break;
        case 0xC4:  // CALL NZ, a16
            callConditional(0);
            break;
        case 0xC5:  // PUSH BC
            tick(4);
            push16(registers_.bc());
            break;
        case 0xC6:  // ADD A, d8
            add(fetch8(), false);
            break;
        case 0xC7:  // RST 0x00
            rst(0x0000);
            break;
        case 0xC8:  // RET Z
            retConditional(1);
            break;
        case 0xC9:  // RET
            ret();
            break;
        case 0xCA:  // JP Z, a16
            jpConditional(1);
            break;
        case 0xCB:  // CB prefix
            executeCb(fetch8());
            break;
        case 0xCC:  // CALL Z, a16
            callConditional(1);
            break;
        case 0xCD: {  // CALL a16
            const uint16_t address = fetch16();
            const uint16_t returnAddress = registers_.pc;
            tick(4);
            push16(returnAddress);
            registers_.pc = address;
            break;
        }
        case 0xCE:  // ADC A, d8
            add(fetch8(), true);
            break;
        case 0xCF:  // RST 0x08
            rst(0x0008);
            break;

        case 0xD0:  // RET NC
            retConditional(2);
            break;
        case 0xD1:  // POP DE
            writeReg16(1, pop16());
            break;
        case 0xD2:  // JP NC, a16
            jpConditional(2);
            break;
        case 0xD3:  // invalid
            invalidOpcode();
            break;
        case 0xD4:  // CALL NC, a16
            callConditional(2);
            break;
        case 0xD5:  // PUSH DE
            tick(4);
            push16(registers_.de());
            break;
        case 0xD6:  // SUB d8
            subtract(fetch8(), false);
            break;
        case 0xD7:  // RST 0x10
            rst(0x0010);
            break;
        case 0xD8:  // RET C
            retConditional(3);
            break;
        case 0xD9:  // RETI
            ret();
            ime_ = true;
            imeDelay_ = 0;
            break;
        case 0xDA:  // JP C, a16
            jpConditional(3);
            break;
        case 0xDB:  // invalid
            invalidOpcode();
            break;
        case 0xDC:  // CALL C, a16
            callConditional(3);
            break;
        case 0xDD:  // invalid
            invalidOpcode();
            break;
        case 0xDE:  // SBC A, d8
            subtract(fetch8(), true);
            break;
        case 0xDF:  // RST 0x18
            rst(0x0018);
            break;

        case 0xE0: {  // LD (0xFF00 + a8), A
            const uint16_t address = static_cast<uint16_t>(0xFF00 | fetch8());
            write8(address, registers_.a);
            break;
        }
        case 0xE1:  // POP HL
            writeReg16(2, pop16());
            break;
        case 0xE2:  // LD (C), A
            write8(static_cast<uint16_t>(0xFF00 + registers_.c), registers_.a);
            break;
        case 0xE3:  // invalid
            invalidOpcode();
            break;
        case 0xE4:  // invalid
            invalidOpcode();
            break;
        case 0xE5:  // PUSH HL
            tick(4);
            push16(registers_.hl());
            break;
        case 0xE6:  // AND d8
            andWith(fetch8());
            break;
        case 0xE7:  // RST 0x20
            rst(0x0020);
            break;
        case 0xE8: {  // ADD SP, e8
            const int8_t offset = static_cast<int8_t>(fetch8());
            addSp(offset);
            tick(8);
            break;
        }
        case 0xE9:  // JP HL
            registers_.pc = registers_.hl();
            break;
        case 0xEA: {  // LD (a16), A
            const uint16_t address = fetch16();
            write8(address, registers_.a);
            break;
        }
        case 0xEB:  // invalid
            invalidOpcode();
            break;
        case 0xEC:  // invalid
            invalidOpcode();
            break;
        case 0xED:  // invalid
            invalidOpcode();
            break;
        case 0xEE:  // XOR d8
            xorWith(fetch8());
            break;
        case 0xEF:  // RST 0x28
            rst(0x0028);
            break;

        case 0xF0: {  // LD A, (0xFF00 + a8)
            const uint16_t address = static_cast<uint16_t>(0xFF00 | fetch8());
            registers_.a = read8(address);
            break;
        }
        case 0xF1:  // POP AF
            registers_.setAf(pop16());
            break;
        case 0xF2:  // LD A, (C)
            registers_.a = read8(static_cast<uint16_t>(0xFF00 + registers_.c));
            break;
        case 0xF3:  // DI
            ime_ = false;
            imeDelay_ = 0;
            break;
        case 0xF4:  // invalid
            invalidOpcode();
            break;
        case 0xF5:  // PUSH AF
            tick(4);
            push16(registers_.af());
            break;
        case 0xF6:  // OR d8
            orWith(fetch8());
            break;
        case 0xF7:  // RST 0x30
            rst(0x0030);
            break;
        case 0xF8: {  // LD HL, SP + e8
            const int8_t offset = static_cast<int8_t>(fetch8());
            loadHlFromSpOffset(offset);
            tick(4);
            break;
        }
        case 0xF9:  // LD SP, HL
            registers_.sp = registers_.hl();
            tick(4);
            break;
        case 0xFA: {  // LD A, (a16)
            const uint16_t address = fetch16();
            registers_.a = read8(address);
            break;
        }
        case 0xFB:  // EI
            // Repeated EI instructions must not postpone the enable forever:
            // the originally scheduled one-instruction delay still expires.
            if (imeDelay_ == 0) {
                imeDelay_ = 2;
            }
            break;
        case 0xFC:  // invalid
            invalidOpcode();
            break;
        case 0xFD:  // invalid
            invalidOpcode();
            break;
        case 0xFE:  // CP d8
            compare(fetch8());
            break;
        case 0xFF:  // RST 0x38
            rst(0x0038);
            break;

        default:
            invalidOpcode();
            break;
    }
}

void GbCpu::executeCb(uint8_t opcode)
{
    const uint8_t group = static_cast<uint8_t>(opcode >> 6);
    const uint8_t bitIndex = static_cast<uint8_t>((opcode >> 3) & 0x07);
    const uint8_t index = static_cast<uint8_t>(opcode & 0x07);

    switch (group) {
        case 0:
            switch (bitIndex) {
                case 0: rotateLeft(index); break;
                case 1: rotateRight(index); break;
                case 2: rotateLeftThroughCarry(index); break;
                case 3: rotateRightThroughCarry(index); break;
                case 4: shiftLeftArithmetic(index); break;
                case 5: shiftRightArithmetic(index); break;
                case 6: swap(index); break;
                default: shiftRightLogical(index); break;
            }
            break;
        case 1:
            bit(bitIndex, index);
            break;
        case 2:
            res(bitIndex, index);
            break;
        default:
            set(bitIndex, index);
            break;
    }
}

void GbCpu::executeAlu(uint8_t operation, uint8_t value)
{
    switch (operation) {
        case 0: add(value, false); break;
        case 1: add(value, true); break;
        case 2: subtract(value, false); break;
        case 3: subtract(value, true); break;
        case 4: andWith(value); break;
        case 5: xorWith(value); break;
        case 6: orWith(value); break;
        default: compare(value); break;
    }
}

void GbCpu::executeLoad(uint8_t destination, uint8_t source)
{
    const uint8_t value = readReg(source);
    writeReg(destination, value);
}

void GbCpu::invalidOpcode()
{
    // Real hardware locks up.  Keeping the emulator alive and counting the
    // event lets a test ROM report a useful failure instead of hanging.
    ++invalidOpcodeCount_;
}

void GbCpu::add(uint8_t value, bool withCarry)
{
    const uint8_t carry = (withCarry && flag(CpuFlags::carry)) ? 1 : 0;
    const uint16_t result = static_cast<uint16_t>(registers_.a)
        + value + carry;
    setHalfCarryFlag((static_cast<uint8_t>(registers_.a & 0x0F)
        + static_cast<uint8_t>(value & 0x0F) + carry) > 0x0F);
    setCarryFlag(result > 0xFF);
    registers_.a = static_cast<uint8_t>(result);
    setZeroFlag(registers_.a);
    setSubtractFlag(false);
}

void GbCpu::subtract(uint8_t value, bool withCarry)
{
    const uint8_t carry = (withCarry && flag(CpuFlags::carry)) ? 1 : 0;
    const int result = static_cast<int>(registers_.a) - static_cast<int>(value) - carry;
    setHalfCarryFlag(static_cast<int>(registers_.a & 0x0F)
        < static_cast<int>(value & 0x0F) + carry);
    setCarryFlag(result < 0);
    registers_.a = static_cast<uint8_t>(result);
    setZeroFlag(registers_.a);
    setSubtractFlag(true);
}

void GbCpu::compare(uint8_t value)
{
    const int result = static_cast<int>(registers_.a) - static_cast<int>(value);
    setHalfCarryFlag(static_cast<int>(registers_.a & 0x0F) < static_cast<int>(value & 0x0F));
    setCarryFlag(result < 0);
    setZeroFlag(static_cast<uint8_t>(result));
    setSubtractFlag(true);
}

void GbCpu::andWith(uint8_t value)
{
    registers_.a &= value;
    setZeroFlag(registers_.a);
    setHalfCarryFlag(true);
    setSubtractFlag(false);
    setCarryFlag(false);
}

void GbCpu::xorWith(uint8_t value)
{
    registers_.a ^= value;
    setZeroFlag(registers_.a);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setCarryFlag(false);
}

void GbCpu::orWith(uint8_t value)
{
    registers_.a |= value;
    setZeroFlag(registers_.a);
    setHalfCarryFlag(false);
    setSubtractFlag(false);
    setCarryFlag(false);
}

void GbCpu::addHl(uint16_t value)
{
    const uint16_t hl = registers_.hl();
    const uint32_t result = static_cast<uint32_t>(hl) + value;
    setHalfCarryFlag((static_cast<uint16_t>(hl & 0x0FFF)
        + static_cast<uint16_t>(value & 0x0FFF)) > 0x0FFF);
    setCarryFlag(result > 0xFFFF);
    setSubtractFlag(false);
    registers_.setHl(static_cast<uint16_t>(result));
    tick(4);
}

void GbCpu::addSp(int8_t offset)
{
    const uint16_t oldSp = registers_.sp;
    const uint8_t offsetByte = static_cast<uint8_t>(offset);
    setHalfCarryFlag((static_cast<uint8_t>(oldSp & 0x0F)
        + static_cast<uint8_t>(offsetByte & 0x0F)) > 0x0F);
    setCarryFlag((static_cast<uint16_t>(oldSp & 0x00FF) + offsetByte) > 0xFF);
    setFlag(CpuFlags::zero, false);
    setSubtractFlag(false);
    registers_.sp = static_cast<uint16_t>(oldSp + offset);
}

void GbCpu::loadHlFromSpOffset(int8_t offset)
{
    const uint16_t oldSp = registers_.sp;
    const uint8_t offsetByte = static_cast<uint8_t>(offset);
    setHalfCarryFlag((static_cast<uint8_t>(oldSp & 0x0F)
        + static_cast<uint8_t>(offsetByte & 0x0F)) > 0x0F);
    setCarryFlag((static_cast<uint16_t>(oldSp & 0x00FF) + offsetByte) > 0xFF);
    setFlag(CpuFlags::zero, false);
    setSubtractFlag(false);
    registers_.setHl(static_cast<uint16_t>(oldSp + offset));
}

uint8_t GbCpu::increment(uint8_t value)
{
    setHalfCarryFlag((value & 0x0F) == 0x0F);
    ++value;
    setZeroFlag(value);
    setSubtractFlag(false);
    return value;
}

uint8_t GbCpu::decrement(uint8_t value)
{
    setHalfCarryFlag((value & 0x0F) == 0x00);
    --value;
    setZeroFlag(value);
    setSubtractFlag(true);
    return value;
}

void GbCpu::daa()
{
    uint8_t correction = 0;
    bool carry = flag(CpuFlags::carry);
    const bool subtractMode = flag(CpuFlags::subtract);

    if (flag(CpuFlags::halfCarry) || (!subtractMode && (registers_.a & 0x0F) > 0x09)) {
        correction = 0x06;
    }
    if (carry || (!subtractMode && registers_.a > 0x99)) {
        correction = static_cast<uint8_t>(correction | 0x60);
        carry = true;
    }

    if (subtractMode) {
        registers_.a = static_cast<uint8_t>(registers_.a - correction);
    } else {
        registers_.a = static_cast<uint8_t>(registers_.a + correction);
    }

    setZeroFlag(registers_.a);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::cpl()
{
    registers_.a = static_cast<uint8_t>(~registers_.a);
    setSubtractFlag(true);
    setHalfCarryFlag(true);
}

void GbCpu::scf()
{
    setCarryFlag(true);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

void GbCpu::ccf()
{
    setCarryFlag(!flag(CpuFlags::carry));
    setSubtractFlag(false);
    setHalfCarryFlag(false);
}

void GbCpu::rlca()
{
    const bool carry = (registers_.a & 0x80) != 0;
    registers_.a = static_cast<uint8_t>((registers_.a << 1) | (carry ? 1 : 0));
    setFlag(CpuFlags::zero, false);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::rrca()
{
    const bool carry = (registers_.a & 0x01) != 0;
    registers_.a = static_cast<uint8_t>((registers_.a >> 1) | (carry ? 0x80 : 0));
    setFlag(CpuFlags::zero, false);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::rla()
{
    const bool oldCarry = flag(CpuFlags::carry);
    const bool newCarry = (registers_.a & 0x80) != 0;
    registers_.a = static_cast<uint8_t>((registers_.a << 1) | (oldCarry ? 1 : 0));
    setFlag(CpuFlags::zero, false);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(newCarry);
}

void GbCpu::rra()
{
    const bool oldCarry = flag(CpuFlags::carry);
    const bool newCarry = (registers_.a & 0x01) != 0;
    registers_.a = static_cast<uint8_t>((registers_.a >> 1) | (oldCarry ? 0x80 : 0));
    setFlag(CpuFlags::zero, false);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(newCarry);
}

void GbCpu::rotateLeft(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool carry = (value & 0x80) != 0;
    value = static_cast<uint8_t>((value << 1) | (carry ? 1 : 0));
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::rotateRight(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool carry = (value & 0x01) != 0;
    value = static_cast<uint8_t>((value >> 1) | (carry ? 0x80 : 0));
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::rotateLeftThroughCarry(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool oldCarry = flag(CpuFlags::carry);
    const bool carry = (value & 0x80) != 0;
    value = static_cast<uint8_t>((value << 1) | (oldCarry ? 1 : 0));
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::rotateRightThroughCarry(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool oldCarry = flag(CpuFlags::carry);
    const bool carry = (value & 0x01) != 0;
    value = static_cast<uint8_t>((value >> 1) | (oldCarry ? 0x80 : 0));
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::shiftLeftArithmetic(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool carry = (value & 0x80) != 0;
    value = static_cast<uint8_t>(value << 1);
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::shiftRightArithmetic(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool carry = (value & 0x01) != 0;
    value = static_cast<uint8_t>((value >> 1) | (value & 0x80));
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::shiftRightLogical(uint8_t index)
{
    uint8_t value = readReg(index);
    const bool carry = (value & 0x01) != 0;
    value = static_cast<uint8_t>(value >> 1);
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(carry);
}

void GbCpu::swap(uint8_t index)
{
    uint8_t value = readReg(index);
    value = static_cast<uint8_t>((value << 4) | (value >> 4));
    writeReg(index, value);
    setZeroFlag(value);
    setSubtractFlag(false);
    setHalfCarryFlag(false);
    setCarryFlag(false);
}

void GbCpu::bit(uint8_t bitIndex, uint8_t index)
{
    const uint8_t value = readReg(index);
    setZeroFlag(static_cast<uint8_t>(value & (1U << bitIndex)));
    setSubtractFlag(false);
    setHalfCarryFlag(true);
}

void GbCpu::res(uint8_t bitIndex, uint8_t index)
{
    const uint8_t value = static_cast<uint8_t>(readReg(index) & ~(1U << bitIndex));
    writeReg(index, value);
}

void GbCpu::set(uint8_t bitIndex, uint8_t index)
{
    const uint8_t value = static_cast<uint8_t>(readReg(index) | (1U << bitIndex));
    writeReg(index, value);
}

}  // namespace qnemu
