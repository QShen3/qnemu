/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <condition_variable>
#include <memory>
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
    ~RasterDisplay() override = default;

    void paintEvent(QPaintEvent* event) override;

    void requestUpdate() override;
    void waitForUpdateFinished() override;

    QImage& getBuffer() override;

private:
    void run();

    QImage buffer;
    std::mutex bufferMutex;
    std::condition_variable cv;
    bool refreshRequested;
    std::thread work;
};

}  // namespace qnemu