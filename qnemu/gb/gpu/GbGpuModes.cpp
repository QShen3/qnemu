/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/gpu/GbGpu.h"

namespace qnemu
{

void GbGpu::mode0()
{
    if (registers.lcdYCoordinate == 143) {
        registers.modeFlag = 1;
        if (display) {
            display->requestRefresh();
        }
        interruptHandler->requestVBlankInterrupt();
        if (registers.mode0HBlankInterrupt) {
            interruptHandler->requestLcdInterrupt();
        }
    } else {
        registers.modeFlag = 2;
        if (registers.mode2OAMInterrupt) {
            interruptHandler->requestLcdInterrupt();
        }
    }
    registers.lcdYCoordinate++;
    checklcdYCoordinate();
}

void GbGpu::mode1()
{
    if (registers.lcdYCoordinate == 153) {
        registers.lcdYCoordinate = 0;
        windowLineCounter = 0;
        registers.modeFlag = 2;
        display->waitFroRefresh();
    }
    else {
        registers.modeFlag = 1;
        registers.lcdYCoordinate++;
    }
    checklcdYCoordinate();
}

void GbGpu::mode2()
{
    scanSprites();
    registers.modeFlag = 3;
}

void GbGpu::mode3()
{
    renderLine();
    registers.modeFlag = 0;
    if (registers.mode0HBlankInterrupt) {
        interruptHandler->requestLcdInterrupt();
    }
}

}  // namespace qnemu