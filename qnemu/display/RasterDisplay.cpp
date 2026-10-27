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

void RasterDisplay::setButtonPressCallback(std::function<void(GbButton)> callback)
{
    buttonPressCallback = std::move(callback);
}

void RasterDisplay::setButtonReleaseCallback(std::function<void(GbButton)> callback)
{
    buttonReleaseCallback = std::move(callback);
}

std::optional<GbButton> RasterDisplay::buttonForQtKey(int key)
{
    switch (key) {
    case Qt::Key_W:
        return GbButton::up;
    case Qt::Key_S:
        return GbButton::down;
    case Qt::Key_A:
        return GbButton::left;
    case Qt::Key_D:
        return GbButton::right;
    case Qt::Key_J:
        return GbButton::a;
    case Qt::Key_K:
        return GbButton::b;
    case Qt::Key_2:
        return GbButton::select;
    case Qt::Key_3:
        return GbButton::start;
    default:
        return std::nullopt;
    }
}

void RasterDisplay::keyPressEvent(QKeyEvent* event)
{
    const auto button = buttonForQtKey(event->key());
    if (button && buttonPressCallback) {
        buttonPressCallback(*button);
    }
}

void RasterDisplay::keyReleaseEvent(QKeyEvent* event)
{
    const auto button = buttonForQtKey(event->key());
    if (button && buttonReleaseCallback) {
        buttonReleaseCallback(*button);
    }
}

}  // namespace qnemu
