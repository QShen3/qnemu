# QNEmu

[![pipeline status](https://gitlab.com/QShen3/qnemu/badges/main/pipeline.svg)](https://gitlab.com/QShen3/qnemu/-/commits/main)

QNEmu is an emulator that support Nintendo Game Boy and Game Boy Color. It is still under development and plans to support other consoles in the future. The main purpose of this project is to study computer architecture and software design pattern.


## Build

Requirements:
- GCC >= 9
- CMake >= 3.13
- Qt >= 6.2
- Google Test >= 1.10

Build command:

    mkdir build && cd build
    cmake .. && make

The executable will be available in qneme/qnemu. To run the test:

    make test
