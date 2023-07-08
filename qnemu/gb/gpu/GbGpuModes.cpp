/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstdint>

#include "qnemu/gb/gpu/GbGpu.h"

namespace qnemu
{

void GbGpu::mode0()
{
    render->horizontalBlank();
}

void GbGpu::mode1()
{
    render->verticalBlank();
}

void GbGpu::mode2()
{
    render->scanSprites();
}

void GbGpu::mode3()
{
    render->renderLine();
}

}  // namespace qnemu