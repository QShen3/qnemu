/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>
#include <mutex>

#include <QtGui/QPainter>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent),
    buffer(160, 144, QImage::Format_RGB32),
    refreshRequested(false)
{
    work = std::thread(&RasterDisplay::run, this);
}

void RasterDisplay::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(0, 0, buffer);
    refreshRequested = false;
    cv.notify_one();
}

void RasterDisplay::requestUpdate()
{
    {
        std::lock_guard lock(bufferMutex);
        refreshRequested = true;
    }
    cv.notify_one();
}

void RasterDisplay::waitForUpdateFinished()
{
    std::unique_lock lock(bufferMutex);
    cv.wait(lock, [this] { return !refreshRequested; });
}

QImage& RasterDisplay::getBuffer()
{
    return buffer;
}

void RasterDisplay::run()
{
    while (true)
    {
        {
            std::unique_lock lock(bufferMutex);
            cv.wait(lock, [this] { return refreshRequested; });
        }
        update();
    }
}

} // namespace qnemu