/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <cmath>
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

EmulatorItem::EmulatorItem(QQuickItem* parent) : QQuickPaintedItem(parent),
    systemAccelerometer_(this)
{
    setFlag(QQuickItem::ItemIsFocusScope, true);
    setAcceptedMouseButtons(Qt::NoButton);
    setOpaquePainting(true);
    setFillColor(Qt::black);
    connect(this, &EmulatorItem::frameReady, this, [this]() {
        updatePending.store(false);
        update();
    }, Qt::QueuedConnection);

    systemAccelerometer_.setCallback([this](float x, float y) {
        if (systemAccelerometerEnabled_) {
            emitAccelerometer(x, y);
        }
    });

    setKeyBindings({
        {QStringLiteral("up"), static_cast<int>(Qt::Key_W)},
        {QStringLiteral("down"), static_cast<int>(Qt::Key_S)},
        {QStringLiteral("left"), static_cast<int>(Qt::Key_A)},
        {QStringLiteral("right"), static_cast<int>(Qt::Key_D)},
        {QStringLiteral("a"), static_cast<int>(Qt::Key_J)},
        {QStringLiteral("b"), static_cast<int>(Qt::Key_K)},
        {QStringLiteral("select"), static_cast<int>(Qt::Key_2)},
        {QStringLiteral("start"), static_cast<int>(Qt::Key_3)},
        {QStringLiteral("tiltLeft"), static_cast<int>(Qt::Key_Left)},
        {QStringLiteral("tiltRight"), static_cast<int>(Qt::Key_Right)},
        {QStringLiteral("tiltUp"), static_cast<int>(Qt::Key_Up)},
        {QStringLiteral("tiltDown"), static_cast<int>(Qt::Key_Down)},
    });
}

void EmulatorItem::presentFrame(const FrameBuffer& frame)
{
    presentPixels(frame.pixels.data(), FrameBuffer::width, FrameBuffer::height);
}

void EmulatorItem::presentSgbFrame(const SgbFrameBuffer& frame)
{
    presentPixels(frame.pixels.data(), SgbFrameBuffer::width, SgbFrameBuffer::height);
}

void EmulatorItem::presentPixels(const uint32_t* pixels, int width, int height)
{
    if (!enabled.load()) {
        return;
    }

    {
        const std::lock_guard lock(frameMutex);
        if (frameImage.size() != QSize(width, height)
            || frameImage.format() != QImage::Format_RGB32) {
            frameImage = QImage(width, height, QImage::Format_RGB32);
        }
        for (int y = 0; y < height; ++y) {
            std::memcpy(frameImage.scanLine(y),
                pixels + static_cast<std::size_t>(y) * width,
                static_cast<std::size_t>(width) * sizeof(uint32_t));
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

void EmulatorItem::setAccelerometerCallback(std::function<void(float, float)> callback)
{
    accelerometerCallback = std::move(callback);
}

void EmulatorItem::setSystemAccelerometerEnabled(bool enabled)
{
    if (systemAccelerometerEnabled_ == enabled) {
        return;
    }
    systemAccelerometerEnabled_ = enabled;
    systemAccelerometer_.setEnabled(enabled);
    if (!enabled) {
        updateKeyboardTilt();
    } else {
        emitAccelerometer(0.0F, 0.0F);
    }
}

QImage EmulatorItem::snapshot() const
{
    const std::lock_guard lock(frameMutex);
    return frameImage;
}

int EmulatorItem::frameWidth() const
{
    const std::lock_guard lock(frameMutex);
    return hasFrame ? frameImage.width() : 160;
}

int EmulatorItem::frameHeight() const
{
    const std::lock_guard lock(frameMutex);
    return hasFrame ? frameImage.height() : 144;
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

    tiltBindings.clear();
    const auto addTilt = [this, &bindings](const QString& name, int axis) {
        const int key = bindings.value(name).toInt();
        if (key != 0) {
            tiltBindings.insert(key, axis);
        }
    };
    addTilt(QStringLiteral("tiltLeft"), 0x1);
    addTilt(QStringLiteral("tiltRight"), 0x2);
    addTilt(QStringLiteral("tiltUp"), 0x4);
    addTilt(QStringLiteral("tiltDown"), 0x8);
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

    const bool hadTilt = !pressedTiltKeys.isEmpty();
    pressedTiltKeys.clear();
    if (hadTilt && !systemAccelerometerEnabled_) {
        emitAccelerometer(0.0F, 0.0F);
    }
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
    if (tiltBindings.contains(event->key())) {
        if (!systemAccelerometerEnabled_) {
            pressedTiltKeys.insert(event->key());
            updateKeyboardTilt();
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
    if (tiltBindings.contains(event->key())) {
        if (!systemAccelerometerEnabled_) {
            pressedTiltKeys.remove(event->key());
            updateKeyboardTilt();
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

int EmulatorItem::tiltForQtKey(int key) const
{
    return tiltBindings.value(key, 0);
}

void EmulatorItem::updateKeyboardTilt()
{
    int axes = 0;
    for (int key : std::as_const(pressedTiltKeys)) {
        axes |= tiltForQtKey(key);
    }
    float x = ((axes & 0x1) ? 1.0F : 0.0F) - ((axes & 0x2) ? 1.0F : 0.0F);
    float y = ((axes & 0x4) ? 1.0F : 0.0F) - ((axes & 0x8) ? 1.0F : 0.0F);
    const float length = std::hypot(x, y);
    if (length > 1.0F) {
        x /= length;
        y /= length;
    }
    emitAccelerometer(x, y);
}

void EmulatorItem::emitAccelerometer(float x, float y)
{
    if (accelerometerCallback) {
        accelerometerCallback(x, y);
    }
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
