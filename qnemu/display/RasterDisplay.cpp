/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include <QtGui/QPainter>
#include <QtGui/QPaintDevice>
#include <QtGui/QResizeEvent>
#include <QtGui/QWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QWindow(parent),
    backingStore(std::make_unique<QBackingStore>(this))
{
}

void RasterDisplay::update(const QImage& image)
{
    if (!isExposed()) {
        return;
    }

    QRect rect(0, 0, width(), height());
    backingStore->beginPaint(rect);

    QPaintDevice* device = backingStore->paintDevice();
    QPainter painter(device);

    painter.drawImage(0, 0, image);

    painter.end();

    backingStore->endPaint();
    backingStore->flush(rect);
}

void RasterDisplay::resizeEvent(QResizeEvent* event)
{
    backingStore->resize(event->size());
}

}  // namespace qnemu