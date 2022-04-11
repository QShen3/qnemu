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

    std::unique_lock<std::mutex> sync() override;

    QImage& getBuffer() override;

signals:
    void requestRefresh() override;

private:
    QImage buffer;
    std::mutex mutex;
};

}  // namespace qnemu