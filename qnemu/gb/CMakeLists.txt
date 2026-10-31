# Each hardware block is a separate Qt-free static library.  Keeping them
# apart lets the unit tests and the headless ROM runner link only the devices
# they exercise; the `gb` library at the end assembles the complete system.
function(qnemu_add_gb_library target)
    add_library(${target} STATIC ${ARGN})
    set_target_properties(${target} PROPERTIES
        AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    target_include_directories(${target} PUBLIC ${PROJECT_SOURCE_DIR})
    target_link_libraries(${target} PRIVATE qnemu_warnings)
endfunction()

# Bundled QNEMU CGB boot ROM.  The binary is committed under third_party and
# embedded here at configure time, so the normal build does not need RGBDS.
set(QNEMU_CGB_BOOT_ROM_BIN
    "${PROJECT_SOURCE_DIR}/third_party/sameboy/bootroms/cgb_boot.bin")
if(NOT EXISTS "${QNEMU_CGB_BOOT_ROM_BIN}")
    message(FATAL_ERROR "Missing CGB boot ROM: ${QNEMU_CGB_BOOT_ROM_BIN}")
endif()
file(READ "${QNEMU_CGB_BOOT_ROM_BIN}" QNEMU_CGB_BOOT_ROM_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1, "
    QNEMU_CGB_BOOT_ROM_BYTES "${QNEMU_CGB_BOOT_ROM_HEX}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/boot/GbCgbBootRom.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/GbCgbBootRom.cpp"
    @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${QNEMU_CGB_BOOT_ROM_BIN}")
qnemu_add_gb_library(gb_bootrom
    "${CMAKE_CURRENT_BINARY_DIR}/GbCgbBootRom.cpp")

# SM83 CPU core.
qnemu_add_gb_library(gb_cpu
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu/GbCpu.cpp)

# Interrupt controller (IF/IE, priority, edge/latch handling).
qnemu_add_gb_library(gb_interrupt
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt/GbInterruptHandler.cpp)

# PPU: dot clock, VRAM/OAM/palette memory, pixel FIFO and the reference
# scanline renderer.
qnemu_add_gb_library(gb_ppu
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/GbPpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/CgbCompatibilityPalette.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuFifo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuRenderer.cpp)

# Super Game Boy command receiver, palette and attribute state.
qnemu_add_gb_library(gb_sgb
    ${CMAKE_CURRENT_SOURCE_DIR}/sgb/GbSgb.cpp)
target_link_libraries(gb_ppu
    PRIVATE
        gb_sgb)

# APU and its four channels.
qnemu_add_gb_library(gb_apu
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbApu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbLengthTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbVolumeEnvelope.cpp)

# DIV/TIMA timer.
qnemu_add_gb_library(gb_timer
    ${CMAKE_CURRENT_SOURCE_DIR}/timer/GbTimer.cpp)

# Cartridge header parsing and the MBC implementations.
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge/mbc CARTRIDGE_MBC_SRCS)
qnemu_add_gb_library(gb_cartridge
    ${CMAKE_CURRENT_SOURCE_DIR}/cartridge/GbCartridge.cpp
    ${CARTRIDGE_MBC_SRCS})

# Internal RAM.
qnemu_add_gb_library(gb_memory
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/GbHighRam.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memory/GbWorkRam.cpp)

# FF00 joypad matrix.
qnemu_add_gb_library(gb_joypad
    ${CMAKE_CURRENT_SOURCE_DIR}/joypad/GbJoypad.cpp)

# Bus address decoding and the CPU bus adapter.
qnemu_add_gb_library(gb_mmu
    ${CMAKE_CURRENT_SOURCE_DIR}/mmu/GbMmu.cpp)
target_link_libraries(gb_mmu
    PRIVATE
        gb_apu
        gb_joypad
        gb_memory
        gb_timer)

# Complete GB system: owns the CPU/MMU/PPU/APU/timer/joypad/cartridge.  The
# host display, audio sink and input backend are injected by the executable,
# so this library stays independent of Qt Widgets/Multimedia (only the save
# service pulls in Qt Core).
qnemu_add_gb_library(gb
    ${CMAKE_CURRENT_SOURCE_DIR}/Gb.cpp)
target_link_libraries(gb
    PUBLIC
        gb_bootrom
        gb_cpu
        gb_interrupt
        gb_ppu
        gb_apu
        gb_timer
        gb_cartridge
        gb_memory
        gb_joypad
        gb_mmu
        gb_sgb
    PRIVATE
        save)
