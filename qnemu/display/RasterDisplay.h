/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <QtGui/QBackingStore>
#include <QtGui/QImage>
#include <QtGui/QPaintDeviceWindow>
#include <QtGui/QWindow>

#include "qnemu/display/DisplayInterface.h"

namespace qnemu
{

class RasterDisplay : public QWindow, public DisplayInterface
{
    Q_OBJECT
public:
    RasterDisplay(QWindow* parent = nullptr);
    ~RasterDisplay() override = default;

    void update(const QImage& image) override;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    std::unique_ptr<QBackingStore> backingStore;
};

}  // namespace qnemu