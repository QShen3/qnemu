/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QtGui/QImage>

namespace qnemu
{

class DisplayInterface
{
public:
    virtual ~DisplayInterface() = default;

    virtual void requestUpdate() = 0;
    virtual void waitForUpdateFinished() = 0;
};

}  // namespace qnemu