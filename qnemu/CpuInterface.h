/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

namespace qnemu
{

class CpuInterface
{
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
};

}  // namespace qnemu