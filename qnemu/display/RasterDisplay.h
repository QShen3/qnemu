/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <QtGui/QImage>
#include <QtGui/QPaintDeviceWindow>
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

private:

};

}  // namespace qnemu