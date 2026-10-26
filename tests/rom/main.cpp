/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

#include "qnemu/gb/cpu/GbCpu.h"
#include "qnemu/gb/cpu/Registers.h"
#include "tests/rom/GbTestSystem.h"

namespace
{

enum class Result
{
    none,
    pass,
    fail,
};

Result textResult(const std::string& output)
{
    const auto failed = output.rfind("Failed");
    const auto passed = output.rfind("Passed");
    if (failed != std::string::npos && (passed == std::string::npos || failed > passed)) {
        return Result::fail;
    }
    if (passed != std::string::npos) {
        return Result::pass;
    }
    return Result::none;
}

Result mooneyeResult(const qnemu::Registers& registers)
{
    const bool passPattern = registers.a == 0x00
        && registers.b == 0x03 && registers.c == 0x05
        && registers.d == 0x08 && registers.e == 0x0D
        && registers.h == 0x15 && registers.l == 0x22;
    if (passPattern) {
        return Result::pass;
    }

    const bool failPattern = registers.b == 0x42 && registers.c == 0x42
        && registers.d == 0x42 && registers.e == 0x42
        && registers.h == 0x42 && registers.l == 0x42;
    if (failPattern) {
        return Result::fail;
    }

    return Result::none;
}

void printUsage(const char* program)
{
    std::cout << "Usage: " << program << " <rom.gb> [options]\n"
              << "  --model dmg|cgb     initial CPU model (default: cgb)\n"
              << "  --max-cycles N      emulated cycle budget (default: 300000000)\n"
              << "  --max-seconds N     wall-clock budget (default: 60)\n"
              << "  --verbose           stream serial output as it is produced\n";
}

}  // namespace

int main(int argc, char** argv)
{
    std::string romPath;
    std::string model = "cgb";
    uint64_t maxCycles = 300000000;
    uint64_t maxSeconds = 60;
    bool verbose = false;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];
        if (argument == "--model" && index + 1 < argc) {
            model = argv[++index];
        } else if (argument == "--max-cycles" && index + 1 < argc) {
            maxCycles = std::strtoull(argv[++index], nullptr, 10);
        } else if (argument == "--max-seconds" && index + 1 < argc) {
            maxSeconds = std::strtoull(argv[++index], nullptr, 10);
        } else if (argument == "--verbose") {
            verbose = true;
        } else if (argument == "--help" || argument == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (argument.rfind("--", 0) == 0) {
            std::cerr << "Unknown option: " << argument << '\n';
            printUsage(argv[0]);
            return 2;
        } else if (romPath.empty()) {
            romPath = argument;
        } else {
            std::cerr << "Unexpected argument: " << argument << '\n';
            return 2;
        }
    }

    if (romPath.empty() || (model != "dmg" && model != "cgb")) {
        printUsage(argv[0]);
        return 2;
    }

    qnemuTest::GbTestSystem system;
    if (!system.loadRom(romPath, model == "cgb")) {
        std::cerr << "Failed to load ROM: " << romPath << '\n';
        return 2;
    }

    qnemu::GbCpu cpu(system);
    if (model == "dmg") {
        auto& registers = cpu.registers();
        registers.a = 0x01;
        registers.f = 0xB0;
        registers.b = 0x00;
        registers.c = 0x13;
        registers.d = 0x00;
        registers.e = 0xD8;
        registers.h = 0x01;
        registers.l = 0x4D;
        registers.sp = 0xFFFE;
        registers.pc = 0x0100;
    }

    std::cout << "ROM: " << romPath << '\n';
    std::cout << "Model: " << model << '\n';

    const auto start = std::chrono::steady_clock::now();
    std::size_t printedSerial = 0;
    Result result = Result::none;
    uint64_t steps = 0;
    std::array<uint16_t, 32> recentPc{};
    std::array<uint8_t, 32> recentOpcode{};
    std::size_t recentIndex = 0;

    while (cpu.cycles() < maxCycles) {
        const uint16_t pc = cpu.registers().pc;
        const uint8_t opcode = system.read(pc);
        recentPc.at(recentIndex) = pc;
        recentOpcode.at(recentIndex) = opcode;
        recentIndex = (recentIndex + 1) % recentPc.size();

        cpu.step();
        ++steps;

        if (opcode == 0x40) {  // LD B, B: Mooneye's debug breakpoint
            result = mooneyeResult(cpu.registers());
            if (result != Result::none) {
                break;
            }
        }

        if ((steps & 0xFF) == 0) {
            result = textResult(system.serialOutput());
            if (result == Result::none) {
                result = textResult(system.screenText());
            }
            if (result != Result::none) {
                break;
            }
        }

        if (verbose && system.serialOutput().size() > printedSerial) {
            std::cout.write(system.serialOutput().data() + printedSerial,
                static_cast<std::streamsize>(system.serialOutput().size() - printedSerial));
            std::cout.flush();
            printedSerial = system.serialOutput().size();
        }

        if ((steps & 0xFFFFF) == 0) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start).count();
            if (static_cast<uint64_t>(elapsed) >= maxSeconds) {
                break;
            }
        }
    }

    if (result == Result::none) {
        result = textResult(system.serialOutput());
    }
    if (result == Result::none) {
        result = textResult(system.screenText());
    }

    if (result != Result::pass) {
        const std::string screen = system.screenText();
        if (!screen.empty()) {
            std::cout << "--- screen ---\n" << screen
                      << "--------------\n";
        }
    }

    if (result == Result::none) {
        std::cout << "--- last instructions ---\n";
        for (std::size_t index = 0; index < recentPc.size(); ++index) {
            const std::size_t position = (recentIndex + index) % recentPc.size();
            std::cout << "0x" << std::hex << recentPc.at(position)
                      << ": 0x" << static_cast<int>(recentOpcode.at(position))
                      << std::dec << '\n';
        }
        std::cout << "-------------------------\n";
    }

    if (!verbose && !system.serialOutput().empty()) {
        std::cout << "--- serial ---\n" << system.serialOutput();
        if (system.serialOutput().back() != '\n') {
            std::cout << '\n';
        }
        std::cout << "--------------\n";
    }

    const auto& registers = cpu.registers();
    std::cout << "Cycles: " << cpu.cycles() << '\n';
    std::cout << "Instructions: " << steps << '\n';
    std::cout << "PC: 0x" << std::hex << registers.pc << std::dec << '\n';
    std::cout << "Registers: AF=0x" << std::hex << registers.af()
              << " BC=0x" << registers.bc()
              << " DE=0x" << registers.de()
              << " HL=0x" << registers.hl()
              << " SP=0x" << registers.sp << std::dec << '\n';
    if (cpu.invalidOpcodeCount() != 0) {
        std::cout << "Invalid opcodes: " << cpu.invalidOpcodeCount() << '\n';
    }

    if (result == Result::pass) {
        std::cout << "PASS\n";
        return 0;
    }
    if (result == Result::fail) {
        std::cout << "FAIL\n";
        return 1;
    }

    std::cout << "TIMEOUT / UNKNOWN\n";
    return 2;
}
