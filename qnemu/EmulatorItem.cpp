/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include <QtCore/QSize>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>

#include "qnemu/EmulatorItem.h"

namespace qnemu
{

EmulatorItem::EmulatorItem(QQuickItem* parent) : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::NoButton);
    setOpaquePainting(true);
    setFillColor(Qt::black);
    connect(this, &EmulatorItem::frameReady, this, [this]() {
        updatePending.store(false);
        update();
    }, Qt::QueuedConnection);
}

void EmulatorItem::presentFrame(const FrameBuffer& frame)
{
    if (!enabled.load()) {
        return;
    }

    {
        const std::lock_guard lock(frameMutex);
        if (frameImage.size() != QSize(FrameBuffer::width, FrameBuffer::height)
            || frameImage.format() != QImage::Format_RGB32) {
            frameImage = QImage(FrameBuffer::width, FrameBuffer::height,
                QImage::Format_RGB32);
        }
        for (int y = 0; y < FrameBuffer::height; ++y) {
            std::memcpy(frameImage.scanLine(y),
                frame.pixels.data() + static_cast<std::size_t>(y) * FrameBuffer::width,
                FrameBuffer::width * sizeof(uint32_t));
        }
        hasFrame = true;
    }

    if (!updatePending.exchange(true)) {
        emit frameReady();
    }
}

void EmulatorItem::enable()
{
    enabled.store(true);
}

void EmulatorItem::disable()
{
    enabled.store(false);
}

void EmulatorItem::setButtonPressCallback(std::function<void(GbButton)> callback)
{
    buttonPressCallback = std::move(callback);
}

void EmulatorItem::setButtonReleaseCallback(std::function<void(GbButton)> callback)
{
    buttonReleaseCallback = std::move(callback);
}

void EmulatorItem::paint(QPainter* painter)
{
    QImage image;
    {
        const std::lock_guard lock(frameMutex);
        if (!hasFrame) {
            return;
        }
        image = frameImage;
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawImage(contentsBoundingRect(), image);
}

void EmulatorItem::keyPressEvent(QKeyEvent* event)
{
    const auto button = buttonForQtKey(event->key());
    if (button && buttonPressCallback) {
        buttonPressCallback(*button);
        event->accept();
        return;
    }
    QQuickItem::keyPressEvent(event);
}

void EmulatorItem::keyReleaseEvent(QKeyEvent* event)
{
    const auto button = buttonForQtKey(event->key());
    if (button && buttonReleaseCallback) {
        buttonReleaseCallback(*button);
        event->accept();
        return;
    }
    QQuickItem::keyReleaseEvent(event);
}

std::optional<GbButton> EmulatorItem::buttonForQtKey(int key)
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

}  // namespace qnemu
