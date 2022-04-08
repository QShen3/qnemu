/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <memory>

#include <QtGui/QPaintDevice>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QRasterWindow>

#include "qnemu/display/RasterDisplay.h"

namespace qnemu
{

RasterDisplay::RasterDisplay(QWindow* parent) :
    QRasterWindow(parent)
{
}

}  // namespace qnemu