/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

void DefaultSound::enable()
{
    isEnabled = true;
}

void DefaultSound::disable()
{
    isEnabled = false;
}

}  // namespace qnemu