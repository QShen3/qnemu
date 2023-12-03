/* 
 *  Copyright [2023] <qazxdrcssc2006@163.com>
 */

#pragma once

namespace qnemu
{

class SoundInterface
{
public:
    virtual ~SoundInterface() = default;

    virtual void enable() = 0;
    virtual void disable() = 0;


};

}  // namespace qnemu