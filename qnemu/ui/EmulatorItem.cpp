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

#include "qnemu/ui/EmulatorItem.h"

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

    setKeyBindings({
        {QStringLiteral("up"), static_cast<int>(Qt::Key_W)},
        {QStringLiteral("down"), static_cast<int>(Qt::Key_S)},
        {QStringLiteral("left"), static_cast<int>(Qt::Key_A)},
        {QStringLiteral("right"), static_cast<int>(Qt::Key_D)},
        {QStringLiteral("a"), static_cast<int>(Qt::Key_J)},
        {QStringLiteral("b"), static_cast<int>(Qt::Key_K)},
        {QStringLiteral("select"), static_cast<int>(Qt::Key_2)},
        {QStringLiteral("start"), static_cast<int>(Qt::Key_3)},
    });
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

QImage EmulatorItem::snapshot() const
{
    const std::lock_guard lock(frameMutex);
    return frameImage;
}

void EmulatorItem::setKeyBindings(const QVariantMap& bindings)
{
    releaseAllButtons();
    keyBindings.clear();
    const auto add = [this, &bindings](const QString& name, GbButton button) {
        const int key = bindings.value(name).toInt();
        if (key != 0) {
            keyBindings.insert(key, button);
        }
    };
    add(QStringLiteral("up"), GbButton::up);
    add(QStringLiteral("down"), GbButton::down);
    add(QStringLiteral("left"), GbButton::left);
    add(QStringLiteral("right"), GbButton::right);
    add(QStringLiteral("a"), GbButton::a);
    add(QStringLiteral("b"), GbButton::b);
    add(QStringLiteral("select"), GbButton::select);
    add(QStringLiteral("start"), GbButton::start);
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

void EmulatorItem::releaseAllButtons()
{
    for (int key : std::as_const(pressedKeys)) {
        const auto button = buttonForQtKey(key);
        if (button && buttonReleaseCallback) {
            buttonReleaseCallback(*button);
        }
    }
    pressedKeys.clear();
}

void EmulatorItem::keyPressEvent(QKeyEvent* event)
{
    const auto button = buttonForQtKey(event->key());
    if (button) {
        pressedKeys.insert(event->key());
        if (buttonPressCallback) {
            buttonPressCallback(*button);
        }
        event->accept();
        return;
    }
    QQuickItem::keyPressEvent(event);
}

void EmulatorItem::keyReleaseEvent(QKeyEvent* event)
{
    const auto button = buttonForQtKey(event->key());
    if (button) {
        pressedKeys.remove(event->key());
        if (buttonReleaseCallback) {
            buttonReleaseCallback(*button);
        }
        event->accept();
        return;
    }
    QQuickItem::keyReleaseEvent(event);
}

void EmulatorItem::focusOutEvent(QFocusEvent* event)
{
    releaseAllButtons();
    QQuickItem::focusOutEvent(event);
}

std::optional<GbButton> EmulatorItem::buttonForQtKey(int key) const
{
    const auto it = keyBindings.constFind(key);
    if (it == keyBindings.constEnd()) {
        return std::nullopt;
    }
    return it.value();
}

}  // namespace qnemu
