/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <atomic>
#include <chrono>

#include <QtCore/QRect>
#include <QtGui/QPainter>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent),
    refreshRequested(false),
    buffer(160, 144, QImage::Format_RGB32)
{
    connect(this, SIGNAL(startRefresh()), this, SLOT(update()));
}

void RasterDisplay::paintEvent(QPaintEvent*)
{
    QRect rect(0, 0, width(), height());
    QPainter painter(this);
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return refreshRequested.load(); });
    painter.drawImage(rect, buffer);
    refreshRequested.store(false);
    cv.notify_all();
}

void RasterDisplay::requestRefresh()
{
    refreshRequested.store(true);
    cv.notify_all();
    emit startRefresh();
}

void RasterDisplay::waitFroRefresh()
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::milliseconds(10), [this] { return !refreshRequested.load(); });
}

QImage& RasterDisplay::getBuffer()
{
    return buffer;
}

} // namespace qnemu