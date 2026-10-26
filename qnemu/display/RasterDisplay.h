/*
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <functional>
#include <mutex>

#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QRasterWindow>

#include "qnemu/display/DisplayInterface.h"
#include "qnemu/display/FrameBuffer.h"

namespace qnemu
{

class RasterDisplay : public QRasterWindow, public DisplayInterface
{
    Q_OBJECT
public:
    explicit RasterDisplay(QWindow* parent = nullptr);
    ~RasterDisplay() override = default;

    void paintEvent(QPaintEvent* event) override;

    void presentFrame(const FrameBuffer& frame) override;
    void enable() override;
    void disable() override;

    void setKeyPressCallback(std::function<void(int)> callback) override;
    void setKeyReleaseCallback(std::function<void(int)> callback) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

signals:
    void startRefresh();

private:
    QImage buffer;
    std::mutex mutex;
    std::function<void(int)> keyPressCallback;
    std::function<void(int)> keyReleaseCallback;
    std::atomic_bool enabled{true};
    bool hasFrame = false;
};

}  // namespace qnemu
