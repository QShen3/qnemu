/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <mutex>

#include <QtGui/QPainter>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent),
    buffer(160, 144, QImage::Format_RGB32)
{
}

void RasterDisplay::paintEvent(QPaintEvent*)
{
    bufferMutex.lock();
    QPainter painter(this);
    painter.drawImage(0, 0, buffer);
    bufferMutex.unlock();
}

void RasterDisplay::requestUpdate()
{
    update();
}

void RasterDisplay::waitForUpdateFinished()
{
    while (true) {
        if (bufferMutex.try_lock()) {
            break;
        }
    }
}

QImage& RasterDisplay::getBuffer()
{
    return buffer;
}

} // namespace qnemu