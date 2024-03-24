/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

#include <QtCore/QRect>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent),
    buffer(160, 144, QImage::Format_RGB32),
    refreshRequested(false),
    started(true)
{
    connect(this, SIGNAL(startRefresh()), this, SLOT(update()));
    work = std::thread(&RasterDisplay::run, this);
}

RasterDisplay::~RasterDisplay()
{
    started.store(false);
    refreshRequested.store(true);
    cv.notify_all();
    if (work.joinable()) {
        work.join();
    }
}

void RasterDisplay::paintEvent(QPaintEvent*)
{
    const QRect rect(0, 0, width(), height());
    QPainter painter(this);
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return refreshRequested.load(); });
    painter.drawImage(rect, buffer);
    refreshRequested.store(false);
    cv.notify_all();
}

void RasterDisplay::enable()
{
    started.store(true);
}

void RasterDisplay::disable()
{
    started.store(false);
}

void RasterDisplay::requestRefresh()
{
    const std::unique_lock<std::mutex> lock(mutex);
    refreshRequested.store(true);
    cv.notify_all();
}

void RasterDisplay::waitFroRefresh()
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::milliseconds(10), [this] { return !refreshRequested.load(); });
    refreshRequested.store(false);
}

QImage& RasterDisplay::getBuffer()
{
    return buffer;
}

void RasterDisplay::setKeyPressCallback(std::function<void(int)> callback)
{
    keyPressCallback = callback;
}

void RasterDisplay::setKeyReleaseCallback(std::function<void(int)> callback)
{
    keyReleaseCallback = callback;
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

void RasterDisplay::run()
{
    while (true) {
        if (!started.load()) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        emit startRefresh();
    }
}

} // namespace qnemu