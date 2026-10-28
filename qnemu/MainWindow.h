/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include "qnemu/gb/Gb.h"

#include <QtCore/QSettings>
#include <QtWidgets/QMainWindow>

namespace qnemu
{

class RasterDisplay;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(GbModel model, QWidget *parent = nullptr);
    ~MainWindow() override;

    void openFile();
    void closeEvent(QCloseEvent *event) override;

private:
    QSettings settings;
    GbModel model;
    std::shared_ptr<RasterDisplay> display;
    std::unique_ptr<Gb> gb;
};

}  // namespace qnemu