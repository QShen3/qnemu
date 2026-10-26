/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <cstring>
#include <functional>
#include <mutex>
#include <utility>

#include <QtCore/QRect>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent),
    buffer(FrameBuffer::width, FrameBuffer::height, QImage::Format_RGB32)
{
    connect(this, SIGNAL(startRefresh()), this, SLOT(update()));
}

void RasterDisplay::paintEvent(QPaintEvent*)
{
    const QRect rect(0, 0, width(), height());
    QPainter painter(this);
    const std::lock_guard lock(mutex);
    if (!hasFrame) {
        return;
    }
    painter.drawImage(rect, buffer);
}

void RasterDisplay::presentFrame(const FrameBuffer& frame)
{
    if (!enabled.load()) {
        return;
    }

    {
        const std::lock_guard lock(mutex);
        for (int y = 0; y < FrameBuffer::height; ++y) {
            std::memcpy(buffer.scanLine(y),
                frame.pixels.data() + static_cast<std::size_t>(y) * FrameBuffer::width,
                FrameBuffer::width * sizeof(uint32_t));
        }
        hasFrame = true;
    }

    emit startRefresh();
}

void RasterDisplay::enable()
{
    enabled.store(true);
}

void RasterDisplay::disable()
{
    enabled.store(false);
}

void RasterDisplay::setKeyPressCallback(std::function<void(int)> callback)
{
    keyPressCallback = std::move(callback);
}

void RasterDisplay::setKeyReleaseCallback(std::function<void(int)> callback)
{
    keyReleaseCallback = std::move(callback);
}

void RasterDisplay::keyPressEvent(QKeyEvent* event)
{
    if (keyPressCallback) {
        keyPressCallback(event->key());
    }
}

void RasterDisplay::keyReleaseEvent(QKeyEvent* event)
{
    if (keyReleaseCallback) {
        keyReleaseCallback(event->key());
    }
}

}  // namespace qnemu
