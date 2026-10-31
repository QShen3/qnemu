/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QVariantMap>
#include <QtGui/QFocusEvent>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtQuick/QQuickPaintedItem>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/input/InputInterface.h"

namespace qnemu
{

/**
 * QML view of the emulator framebuffer.
 *
 * The PPU calls presentFrame() from the emulation thread.  The latest frame is
 * copied under a mutex and a queued signal asks the GUI thread to schedule a
 * repaint.  Input events are translated to GbButton transitions and forwarded
 * to the joypad through InputInterface.
 */
class EmulatorItem : public QQuickPaintedItem, public DisplayInterface, public InputInterface
{
    Q_OBJECT
public:
    explicit EmulatorItem(QQuickItem* parent = nullptr);
    ~EmulatorItem() override = default;

    void presentFrame(const FrameBuffer& frame) override;
    void presentSgbFrame(const SgbFrameBuffer& frame) override;
    void enable() override;
    void disable() override;

    void setButtonPressCallback(std::function<void(GbButton)> callback) override;
    void setButtonReleaseCallback(std::function<void(GbButton)> callback) override;

    /// Replaces the host-key to Game Boy button mapping.
    void setKeyBindings(const QVariantMap& bindings);

    /// Releases every key currently held in this view (window deactivation).
    Q_INVOKABLE void releaseAllButtons();

    /// Returns a copy of the latest presented frame (thread-safe).
    QImage snapshot() const;

signals:
    void frameReady();

protected:
    void paint(QPainter* painter) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void presentPixels(const uint32_t* pixels, int width, int height);
    std::optional<GbButton> buttonForQtKey(int key) const;

    mutable std::mutex frameMutex;
    QImage frameImage;
    bool hasFrame = false;
    std::atomic_bool enabled {true};
    std::atomic_bool updatePending {false};
    QHash<int, GbButton> keyBindings;
    QSet<int> pressedKeys;

    std::function<void(GbButton)> buttonPressCallback;
    std::function<void(GbButton)> buttonReleaseCallback;
};

}  // namespace qnemu
