/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <QtGui/QImage>
#include <QtGui/QRasterWindow>

#include "qnemu/display/DisplayInterface.h"

namespace qnemu
{

class RasterDisplay : public QRasterWindow, public DisplayInterface
{
    Q_OBJECT
public:
    RasterDisplay(QWindow* parent = nullptr);
    ~RasterDisplay();

    void paintEvent(QPaintEvent* event) override;

    void requestRefresh() override;
    void waitFroRefresh() override;

    QImage& getBuffer() override;

signals:
    void startRefresh();

private:
    void run();

    QImage buffer;
    std::condition_variable cv;
    std::mutex mutex;
    std::atomic_bool refreshRequested;
    std::atomic_bool started;
    std::thread work;
};

}  // namespace qnemu