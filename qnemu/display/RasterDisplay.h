/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <optional>

#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QRasterWindow>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/display/FrameBuffer.h"
#include "qnemu/input/InputInterface.h"

namespace qnemu
{

class RasterDisplay : public QRasterWindow, public DisplayInterface, public InputInterface
{
    Q_OBJECT
public:
    explicit RasterDisplay(QWindow* parent = nullptr);
    ~RasterDisplay() override = default;

    void paintEvent(QPaintEvent* event) override;

    void presentFrame(const FrameBuffer& frame) override;
    void enable() override;
    void disable() override;

    void setButtonPressCallback(std::function<void(GbButton)> callback) override;
    void setButtonReleaseCallback(std::function<void(GbButton)> callback) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

signals:
    void startRefresh();

private:
    static std::optional<GbButton> buttonForQtKey(int key);

    QImage buffer;
    std::mutex mutex;
    std::function<void(GbButton)> buttonPressCallback;
    std::function<void(GbButton)> buttonReleaseCallback;
    std::atomic_bool enabled{true};
    bool hasFrame = false;
};

}  // namespace qnemu
