/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <mutex>

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

    QImage buffer;
    std::mutex bufferMutex;
};

}  // namespace qnemu