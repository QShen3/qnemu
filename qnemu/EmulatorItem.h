/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>

#include <QtCore/QObject>
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
    void enable() override;
    void disable() override;

    void setButtonPressCallback(std::function<void(GbButton)> callback) override;
    void setButtonReleaseCallback(std::function<void(GbButton)> callback) override;

signals:
    void frameReady();

protected:
    void paint(QPainter* painter) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:
    static std::optional<GbButton> buttonForQtKey(int key);

    std::mutex frameMutex;
    QImage frameImage;
    bool hasFrame = false;
    std::atomic_bool enabled {true};
    std::atomic_bool updatePending {false};

    std::function<void(GbButton)> buttonPressCallback;
    std::function<void(GbButton)> buttonReleaseCallback;
};

}  // namespace qnemu
