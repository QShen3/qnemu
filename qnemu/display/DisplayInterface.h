/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <functional>
#include <mutex>

#include <QtGui/QImage>

namespace qnemu
{

class DisplayInterface
{
public:
    virtual ~DisplayInterface() = default;

    virtual void requestRefresh() = 0;
    virtual void waitFroRefresh() = 0;

    virtual QImage& getBuffer() = 0;

    virtual void setKeyPressCallback(std::function<void(int)> callback) = 0;
    virtual void setKeyReleaseCallback(std::function<void(int)> callback) = 0;
};

}  // namespace qnemu