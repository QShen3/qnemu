# The SM83 core is deliberately independent of Qt and of the rest of the
# emulator.  Keeping it in its own library makes it linkable by the headless
# ROM test runner and by CPU unit tests without dragging in the GUI.
add_library(gb_cpu STATIC ${CMAKE_CURRENT_SOURCE_DIR}/cpu/GbCpu.cpp)
set_target_properties(gb_cpu PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_cpu PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_cpu PRIVATE qnemu_warnings)

# The PPU and interrupt controller are also Qt-free.  The headless test tools
# and the PPU unit tests link this library directly.
add_library(gb_ppu STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/GbPpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt/GbInterruptHandler.cpp)
set_target_properties(gb_ppu PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_ppu PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_ppu PRIVATE qnemu_warnings)

# The timer is Qt-free too.  The headless ROM runner uses this same
# implementation, so the real emulator and the test bus cannot drift apart.
add_library(gb_timer STATIC ${CMAKE_CURRENT_SOURCE_DIR}/timer/GbTimer.cpp)
set_target_properties(gb_timer PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_timer PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_timer PRIVATE qnemu_warnings)

# The APU is Qt-free as well, so the headless ROM runner and the APU unit
# tests can link it without pulling in the GUI or Qt Multimedia.
add_library(gb_apu STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbApu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbChannel4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbLengthTimer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apu/GbVolumeEnvelope.cpp)
set_target_properties(gb_apu PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_apu PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_apu PRIVATE qnemu_warnings)

# Cartridge header parsing and the MBC implementations are Qt-free; the
# headless runner uses the same cartridge/MBC code as the GUI.
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cartridge/mbc CARTRIDGE_MBC_SRCS)
add_library(gb_cartridge STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cartridge/GbCartridge.cpp
    ${CARTRIDGE_MBC_SRCS})
set_target_properties(gb_cartridge PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(gb_cartridge PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(gb_cartridge PRIVATE qnemu_warnings)

# C++
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/interrupt INTERRUPT_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/memory MEMORY_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/gpu GPU_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/joypad JOYPAD_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/timer TIMER_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/mmu MMU_SRCS)

list(REMOVE_ITEM GPU_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/GbPpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu/PpuRenderer.cpp)
list(REMOVE_ITEM INTERRUPT_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt/GbInterruptHandler.cpp)
list(REMOVE_ITEM TIMER_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/timer/GbTimer.cpp)

add_library(
    gb OBJECT
    Gb.cpp
    ${INTERRUPT_SRCS}
    ${MEMORY_SRCS}
    ${GPU_SRCS}
    ${JOYPAD_SRCS}
    ${TIMER_SRCS}
    ${MMU_SRCS}
)
target_include_directories(gb PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(gb PRIVATE qnemu_warnings gb_cpu gb_ppu gb_apu gb_timer gb_cartridge save Qt6::Core Qt6::Gui)
