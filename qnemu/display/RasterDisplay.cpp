/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <atomic>

#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent),
    buffer(160, 144, QImage::Format_RGB32)
{
    connect(this, SIGNAL(requestRefresh()), this, SLOT(update()));
}

void RasterDisplay::paintEvent(QPaintEvent*)
{
    QRect rect(0, 0, width(), height());
    QPainter painter(this);
    std::unique_lock<std::mutex> lock(mutex);
    painter.drawImage(rect, buffer);
}

std::unique_lock<std::mutex> RasterDisplay::sync()
{
    return std::move(std::unique_lock<std::mutex>(mutex));
}

QImage& RasterDisplay::getBuffer()
{
    return buffer;
}

} // namespace qnemu