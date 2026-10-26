/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>

#include <gtest/gtest.h>

#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/cpu/Registers.h"
#include "tests/support/TestBus.h"

namespace qnemuTest
{

class StopCompletingBus : public TestBus
{
public:
    bool onStop() override { return false; }
};

class GbCpuTest : public testing::Test
{
protected:
    void load(std::initializer_list<uint8_t> bytes, uint16_t address = 0x0100)
    {
        cpu.reset();
        bus.memory.fill(0);
        bus.ticks = 0;

        uint16_t cursor = address;
        for (const uint8_t byte : bytes) {
            bus.memory.at(cursor) = byte;
            ++cursor;
        }
        cpu.registers().pc = address;
    }

    TestBus bus;
    qnemu::GbCpu cpu{bus};
};

TEST_F(GbCpuTest, ResetState)
{
    EXPECT_EQ(cpu.registers().a, 0x11);
    EXPECT_EQ(cpu.registers().f, 0x80);
    EXPECT_EQ(cpu.registers().b, 0x00);
    EXPECT_EQ(cpu.registers().c, 0x00);
    EXPECT_EQ(cpu.registers().d, 0xFF);
    EXPECT_EQ(cpu.registers().e, 0x56);
    EXPECT_EQ(cpu.registers().h, 0x00);
    EXPECT_EQ(cpu.registers().l, 0x0D);
    EXPECT_EQ(cpu.registers().sp, 0xFFFE);
    EXPECT_EQ(cpu.registers().pc, 0x0100);
    EXPECT_FALSE(cpu.interruptsEnabled());
    EXPECT_FALSE(cpu.isHalted());
    EXPECT_FALSE(cpu.isStopped());
    EXPECT_EQ(cpu.cycles(), 0U);
}

TEST_F(GbCpuTest, NopAdvancesPcAndConsumesFourCycles)
{
    load({0x00, 0x00});

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0101);
    EXPECT_EQ(cpu.cycles(), 4U);
    EXPECT_EQ(bus.ticks, 4U);

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0102);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, LdRegisterPairImmediate)
{
    load({0x01, 0x34, 0x12});  // LD BC, 0x1234

    cpu.step();
    EXPECT_EQ(cpu.registers().bc(), 0x1234);
    EXPECT_EQ(cpu.registers().pc, 0x0103);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, JrUsesSignedOffsetForBackwardJump)
{
    load({0x00, 0x18, 0xFC});  // 0x0101: JR -4
    cpu.registers().pc = 0x0101;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x00FF);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, JrNcUsesSignedOffsetForBackwardJump)
{
    load({0x00, 0x30, 0xFC});  // 0x0101: JR NC, -4
    cpu.registers().pc = 0x0101;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x00FF);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, ConditionalJrNotTakenCostsEightCycles)
{
    load({0x30, 0xFC});  // JR NC, -4
    cpu.registers().f = qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0102);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, LdHlFromSpPlusPositiveOffset)
{
    load({0xF8, 0x08});  // LD HL, SP + 8
    cpu.registers().sp = 0xFFF8;

    cpu.step();
    EXPECT_EQ(cpu.registers().hl(), 0x0000);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, LdHlFromSpPlusNegativeOffset)
{
    load({0xF8, 0xF8});  // LD HL, SP - 8
    cpu.registers().sp = 0xFFF8;

    cpu.step();
    EXPECT_EQ(cpu.registers().hl(), 0xFFF0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, AddSpImmediate)
{
    load({0xE8, 0x08});  // ADD SP, 8
    cpu.registers().sp = 0xFFF8;

    cpu.step();
    EXPECT_EQ(cpu.registers().sp, 0x0000);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.cycles(), 16U);
}

TEST_F(GbCpuTest, AddHlPreservesZeroFlag)
{
    load({0x09});  // ADD HL, BC
    cpu.registers().setHl(0x0FFF);
    cpu.registers().setBc(0x0001);
    cpu.registers().f = qnemu::CpuFlags::zero;

    cpu.step();
    EXPECT_EQ(cpu.registers().hl(), 0x1000);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, AddHlCarry)
{
    load({0x09});
    cpu.registers().setHl(0xFFFF);
    cpu.registers().setBc(0x0001);

    cpu.step();
    EXPECT_EQ(cpu.registers().hl(), 0x0000);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
}

TEST_F(GbCpuTest, AdcIncludesCarryWithoutWrappingTheOperand)
{
    load({0xCE, 0x00});  // ADC A, 0
    cpu.registers().a = 0xFF;
    cpu.registers().f = qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x00);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
}

TEST_F(GbCpuTest, AdcWithoutCarryDoesNotAddOne)
{
    load({0xCE, 0x00});  // ADC A, 0
    cpu.registers().a = 0x00;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x00);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
}

TEST_F(GbCpuTest, SbcIncludesCarryWithoutWrappingTheOperand)
{
    load({0xDE, 0x00});  // SBC A, 0
    cpu.registers().a = 0x00;
    cpu.registers().f = qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0xFF);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
}

TEST_F(GbCpuTest, SbcWithoutCarryDoesNotSubtractOne)
{
    load({0xDE, 0x00});  // SBC A, 0
    cpu.registers().a = 0x01;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x01);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
}

TEST_F(GbCpuTest, RotateAccumulatorClearsZeroFlag)
{
    load({0x07});  // RLCA
    cpu.registers().a = 0x00;
    cpu.registers().f = qnemu::CpuFlags::zero;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x00);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
}

TEST_F(GbCpuTest, RlaUsesCarryAndClearsZeroFlag)
{
    load({0x17});  // RLA
    cpu.registers().a = 0x80;
    cpu.registers().f = qnemu::CpuFlags::carry | qnemu::CpuFlags::zero;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x01);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
}

TEST_F(GbCpuTest, DaaAfterAddition)
{
    load({0x27});  // DAA
    cpu.registers().a = 0x0A;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x10);
    EXPECT_EQ(cpu.registers().f, 0);
}

TEST_F(GbCpuTest, DaaProducesCarry)
{
    load({0x27});
    cpu.registers().a = 0x9A;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x00);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
}

TEST_F(GbCpuTest, DaaAfterSubtraction)
{
    load({0x27});
    cpu.registers().a = 0x00;
    cpu.registers().f = qnemu::CpuFlags::subtract
        | qnemu::CpuFlags::halfCarry | qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x9A);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
}

TEST_F(GbCpuTest, PopAfMasksTheLowFlagNibble)
{
    load({0xF1});  // POP AF
    cpu.registers().sp = 0xC000;
    bus.memory[0xC000] = 0xFF;
    bus.memory[0xC001] = 0x12;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x12);
    EXPECT_EQ(cpu.registers().f, 0xF0);
    EXPECT_EQ(cpu.registers().sp, 0xC002);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, PushWritesHighByteFirst)
{
    load({0xC5});  // PUSH BC
    cpu.registers().setBc(0x1234);
    cpu.registers().sp = 0xFFFE;

    cpu.step();
    EXPECT_EQ(cpu.registers().sp, 0xFFFC);
    EXPECT_EQ(bus.memory[0xFFFD], 0x12);
    EXPECT_EQ(bus.memory[0xFFFC], 0x34);
    EXPECT_EQ(cpu.cycles(), 16U);
}

TEST_F(GbCpuTest, CallAndReturn)
{
    load({0xCD, 0x34, 0x12});  // CALL 0x1234
    cpu.registers().sp = 0xFFFE;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x1234);
    EXPECT_EQ(cpu.registers().sp, 0xFFFC);
    EXPECT_EQ(bus.memory[0xFFFD], 0x01);
    EXPECT_EQ(bus.memory[0xFFFC], 0x03);
    EXPECT_EQ(cpu.cycles(), 24U);

    bus.memory[0x1234] = 0xC9;  // RET
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0103);
    EXPECT_EQ(cpu.registers().sp, 0xFFFE);
    EXPECT_EQ(cpu.cycles(), 40U);
}

TEST_F(GbCpuTest, RstPushesTheFollowingInstructionAddress)
{
    load({0xFF});  // RST 0x38
    cpu.registers().sp = 0xFFFE;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0038);
    EXPECT_EQ(cpu.registers().sp, 0xFFFC);
    EXPECT_EQ(bus.memory[0xFFFD], 0x01);
    EXPECT_EQ(bus.memory[0xFFFC], 0x01);
    EXPECT_EQ(cpu.cycles(), 16U);
}

TEST_F(GbCpuTest, CbRlcRotatesAndSetsCarry)
{
    load({0xCB, 0x00});  // RLC B
    cpu.registers().b = 0x80;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().b, 0x01);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, CbBitReadsMemoryAndPreservesCarry)
{
    load({0xCB, 0x7E});  // BIT 7, (HL)
    cpu.registers().setHl(0xC000);
    bus.memory[0xC000] = 0x00;
    cpu.registers().f = qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, CbSetWritesMemory)
{
    load({0xCB, 0xDE});  // SET 3, (HL)
    cpu.registers().setHl(0xC000);
    bus.memory[0xC000] = 0x00;

    cpu.step();
    EXPECT_EQ(bus.memory[0xC000], 0x08);
    EXPECT_EQ(cpu.cycles(), 16U);
}

TEST_F(GbCpuTest, LdHlIncrementAddress)
{
    load({0x22});  // LD (HL+), A
    cpu.registers().setHl(0xC000);
    cpu.registers().a = 0x55;

    cpu.step();
    EXPECT_EQ(bus.memory[0xC000], 0x55);
    EXPECT_EQ(cpu.registers().hl(), 0xC001);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, LdAFromHlDecrementAddress)
{
    load({0x3A});  // LD A, (HL-)
    cpu.registers().setHl(0xC001);
    bus.memory[0xC001] = 0x66;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x66);
    EXPECT_EQ(cpu.registers().hl(), 0xC000);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, LdhUsesHighMemoryPage)
{
    load({0xE0, 0x80});  // LD (0xFF80), A
    cpu.registers().a = 0x42;

    cpu.step();
    EXPECT_EQ(bus.memory[0xFF80], 0x42);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, LdAddressSpWritesBothBytes)
{
    load({0x08, 0x00, 0xC0});  // LD (0xC000), SP
    cpu.registers().sp = 0x1234;

    cpu.step();
    EXPECT_EQ(bus.memory[0xC000], 0x34);
    EXPECT_EQ(bus.memory[0xC001], 0x12);
    EXPECT_EQ(cpu.cycles(), 20U);
}

TEST_F(GbCpuTest, IncMemoryCellTimingAndFlags)
{
    load({0x34});  // INC (HL)
    cpu.registers().setHl(0xC000);
    bus.memory[0xC000] = 0x0F;
    cpu.registers().f = qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_EQ(bus.memory[0xC000], 0x10);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, LdMemoryImmediateTiming)
{
    load({0x36, 0x42});  // LD (HL), 0x42
    cpu.registers().setHl(0xC000);

    cpu.step();
    EXPECT_EQ(bus.memory[0xC000], 0x42);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, CompareFlags)
{
    load({0xFE, 0x10});  // CP 0x10
    cpu.registers().a = 0x10;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x10);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
    EXPECT_EQ(cpu.cycles(), 8U);
}

TEST_F(GbCpuTest, LogicalOperationsSetFlags)
{
    load({0xE6, 0x0F});  // AND 0x0F
    cpu.registers().a = 0xF0;
    cpu.registers().f = qnemu::CpuFlags::carry;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x00);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::zero, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
}

TEST_F(GbCpuTest, ScfAndCcf)
{
    load({0x37, 0x3F});  // SCF; CCF
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::carry, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);

    cpu.step();
    EXPECT_EQ(cpu.registers().f & qnemu::CpuFlags::carry, 0);
}

TEST_F(GbCpuTest, CplSetsSubtractAndHalfCarry)
{
    load({0x2F});
    cpu.registers().a = 0x0F;
    cpu.registers().f = 0;

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0xF0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::subtract, 0);
    EXPECT_NE(cpu.registers().f & qnemu::CpuFlags::halfCarry, 0);
}

TEST_F(GbCpuTest, EiEnablesInterruptsAfterTheFollowingInstruction)
{
    load({0xFB, 0x00, 0x00});  // EI; NOP; NOP
    bus.memory[0xFF0F] = 0x01;
    bus.memory[0xFFFF] = 0x01;

    cpu.step();  // EI
    EXPECT_FALSE(cpu.interruptsEnabled());
    EXPECT_EQ(cpu.registers().pc, 0x0101);

    cpu.step();  // NOP, IME becomes effective after this instruction
    EXPECT_TRUE(cpu.interruptsEnabled());
    EXPECT_EQ(cpu.registers().pc, 0x0102);

    cpu.step();  // interrupt dispatch
    EXPECT_EQ(cpu.registers().pc, 0x0040);
}

TEST_F(GbCpuTest, DiClearsInterruptEnable)
{
    load({0xFB, 0x00, 0xF3, 0x3C});  // EI; NOP; DI; INC A
    cpu.registers().a = 0;

    cpu.step();  // EI
    cpu.step();  // NOP
    EXPECT_TRUE(cpu.interruptsEnabled());

    cpu.step();  // DI
    EXPECT_FALSE(cpu.interruptsEnabled());

    bus.memory[0xFF0F] = 0x01;
    bus.memory[0xFFFF] = 0x01;

    cpu.step();  // must execute INC A, not dispatch the pending interrupt
    EXPECT_EQ(cpu.registers().a, 0x01);
    EXPECT_EQ(cpu.registers().pc, 0x0104);
}

TEST_F(GbCpuTest, RetiEnablesInterruptsImmediately)
{
    load({0xD9});  // RETI
    cpu.registers().sp = 0xFFFC;
    bus.memory[0xFFFC] = 0x00;
    bus.memory[0xFFFD] = 0x02;
    bus.memory[0xFF0F] = 0x01;
    bus.memory[0xFFFF] = 0x01;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0200);
    EXPECT_TRUE(cpu.interruptsEnabled());
    EXPECT_EQ(cpu.cycles(), 16U);

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0040);
}

TEST_F(GbCpuTest, InterruptDispatchPushesPcAndClearsRequest)
{
    load({0xFB, 0x00, 0x00});  // EI; NOP; NOP
    cpu.step();
    cpu.step();

    bus.memory[0xFF0F] = 0x01;
    bus.memory[0xFFFF] = 0x01;
    cpu.registers().pc = 0x1234;
    cpu.registers().sp = 0xFFFE;
    const uint64_t before = cpu.cycles();

    ASSERT_TRUE(cpu.interruptsEnabled());
    ASSERT_EQ(bus.memory[0xFF0F], 0x01);
    ASSERT_EQ(bus.memory[0xFFFF], 0x01);

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0040);
    EXPECT_EQ(cpu.registers().sp, 0xFFFC);
    EXPECT_EQ(bus.memory[0xFFFD], 0x12);
    EXPECT_EQ(bus.memory[0xFFFC], 0x34);
    EXPECT_EQ(bus.memory[0xFF0F], 0x00);
    EXPECT_FALSE(cpu.interruptsEnabled());
    EXPECT_EQ(cpu.cycles() - before, 20U);
}

TEST_F(GbCpuTest, InterruptPriorityUsesLowestBitFirst)
{
    load({0xFB, 0x00, 0x00});  // EI; NOP; NOP
    cpu.step();
    cpu.step();

    bus.memory[0xFF0F] = 0x06;  // LCD + timer
    bus.memory[0xFFFF] = 0x06;
    cpu.registers().pc = 0x1234;

    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0048);  // LCD wins over timer
    EXPECT_EQ(bus.memory[0xFF0F], 0x04);
}

TEST_F(GbCpuTest, HaltWithImeZeroWakesWithoutDispatching)
{
    load({0x76, 0x3C});  // HALT; INC A
    cpu.registers().a = 0;
    bus.memory[0xFF0F] = 0x00;
    bus.memory[0xFFFF] = 0x01;

    cpu.step();
    EXPECT_TRUE(cpu.isHalted());
    EXPECT_EQ(cpu.cycles(), 4U);

    cpu.step();
    EXPECT_TRUE(cpu.isHalted());
    EXPECT_EQ(cpu.cycles(), 8U);

    bus.memory[0xFF0F] = 0x01;
    cpu.step();
    EXPECT_FALSE(cpu.isHalted());
    EXPECT_EQ(cpu.registers().a, 0x01);
    EXPECT_EQ(cpu.registers().pc, 0x0102);
}

TEST_F(GbCpuTest, HaltBugExecutesTheFollowingByteTwice)
{
    load({0x76, 0x3C});  // HALT; INC A
    cpu.registers().a = 0;
    bus.memory[0xFF0F] = 0x01;
    bus.memory[0xFFFF] = 0x01;

    cpu.step();  // HALT with IME=0 and a pending interrupt
    EXPECT_FALSE(cpu.isHalted());

    cpu.step();  // INC A, PC does not advance
    EXPECT_EQ(cpu.registers().a, 0x01);
    EXPECT_EQ(cpu.registers().pc, 0x0101);

    cpu.step();  // INC A again, PC advances
    EXPECT_EQ(cpu.registers().a, 0x02);
    EXPECT_EQ(cpu.registers().pc, 0x0102);
}

TEST_F(GbCpuTest, StopConsumesPaddingAndWaitsForJoypad)
{
    load({0x10, 0x00, 0x3C});  // STOP 0; INC A
    cpu.registers().a = 0;
    cpu.step();
    EXPECT_TRUE(cpu.isStopped());
    EXPECT_EQ(cpu.registers().pc, 0x0102);
    EXPECT_EQ(cpu.cycles(), 8U);

    cpu.step();
    EXPECT_TRUE(cpu.isStopped());
    EXPECT_EQ(cpu.cycles(), 12U);

    bus.memory[0xFF0F] = 0x10;
    cpu.step();
    EXPECT_FALSE(cpu.isStopped());
    EXPECT_EQ(cpu.registers().a, 0x01);
    EXPECT_EQ(cpu.registers().pc, 0x0103);
}

TEST(GbCpuStopTest, SystemCanCompleteStopImmediately)
{
    StopCompletingBus bus;
    qnemu::GbCpu cpu(bus);
    bus.memory[0x0100] = 0x10;  // STOP 0
    bus.memory[0x0101] = 0x00;
    bus.memory[0x0102] = 0x3C;  // INC A

    cpu.step();
    EXPECT_FALSE(cpu.isStopped());
    EXPECT_EQ(cpu.registers().pc, 0x0102);

    cpu.step();
    EXPECT_EQ(cpu.registers().a, 0x12);
    EXPECT_EQ(cpu.registers().pc, 0x0103);
}

TEST_F(GbCpuTest, InvalidOpcodeIsCountedAndTreatedAsNop)
{
    load({0xD3});

    cpu.step();
    EXPECT_EQ(cpu.invalidOpcodeCount(), 1U);
    EXPECT_EQ(cpu.registers().pc, 0x0101);
    EXPECT_EQ(cpu.cycles(), 4U);
}

TEST_F(GbCpuTest, InvalidOpcodeSetMatchesSm83)
{
    constexpr std::array<uint8_t, 11> invalidOpcodes {
        0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4, 0xFC, 0xFD,
    };

    for (uint16_t opcode = 0; opcode < 0x100; ++opcode) {
        load({static_cast<uint8_t>(opcode), 0x00, 0x00, 0x00});
        cpu.registers().setHl(0xC000);
        cpu.registers().sp = 0xFFFE;
        cpu.step();

        const bool expectedInvalid = std::find(
            invalidOpcodes.begin(), invalidOpcodes.end(), opcode) != invalidOpcodes.end();
        EXPECT_EQ(cpu.invalidOpcodeCount(), expectedInvalid ? 1U : 0U)
            << "opcode 0x" << std::hex << opcode;
    }
}

TEST_F(GbCpuTest, ConditionalJumpTimings)
{
    load({0x20, 0x02});  // JR NZ, +2
    cpu.registers().f = qnemu::CpuFlags::zero;
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0102);
    EXPECT_EQ(cpu.cycles(), 8U);

    load({0x20, 0x02});
    cpu.registers().f = 0;
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0104);
    EXPECT_EQ(cpu.cycles(), 12U);
}

TEST_F(GbCpuTest, ConditionalCallAndReturnTimings)
{
    load({0xC4, 0x00, 0x20});  // CALL NZ, 0x2000
    cpu.registers().f = qnemu::CpuFlags::zero;
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0103);
    EXPECT_EQ(cpu.registers().sp, 0xFFFE);
    EXPECT_EQ(cpu.cycles(), 12U);

    load({0xC4, 0x00, 0x20});
    cpu.registers().f = 0;
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x2000);
    EXPECT_EQ(cpu.registers().sp, 0xFFFC);
    EXPECT_EQ(cpu.cycles(), 24U);

    load({0xC0});  // RET NZ
    cpu.registers().f = qnemu::CpuFlags::zero;
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0101);
    EXPECT_EQ(cpu.cycles(), 8U);

    load({0xC0});
    cpu.registers().f = 0;
    cpu.registers().sp = 0xFFFC;
    bus.memory[0xFFFC] = 0x00;
    bus.memory[0xFFFD] = 0x02;
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x0200);
    EXPECT_EQ(cpu.registers().sp, 0xFFFE);
    EXPECT_EQ(cpu.cycles(), 20U);
}

TEST_F(GbCpuTest, JumpAndLoadStackPointerTimings)
{
    load({0xC3, 0x00, 0x20});  // JP 0x2000
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x2000);
    EXPECT_EQ(cpu.cycles(), 16U);

    load({0xF9});  // LD SP, HL
    cpu.registers().setHl(0x1234);
    cpu.step();
    EXPECT_EQ(cpu.registers().sp, 0x1234);
    EXPECT_EQ(cpu.cycles(), 8U);

    load({0xE9});  // JP HL
    cpu.registers().setHl(0x2345);
    cpu.step();
    EXPECT_EQ(cpu.registers().pc, 0x2345);
    EXPECT_EQ(cpu.cycles(), 4U);
}

}  // namespace qnemuTest
