/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <mutex>

#include <QtGui/QImage>

namespace qnemu
{

class DisplayInterface
{
public:
    virtual ~DisplayInterface() = default;

    virtual void requestRefresh() = 0;
    virtual std::unique_lock<std::mutex> sync() = 0;

    virtual QImage& getBuffer() = 0;
};

}  // namespace qnemu