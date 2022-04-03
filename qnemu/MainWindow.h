/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QtCore/QSettings>
#include <QtWidgets/QMainWindow>

namespace qnemu
{

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void openFile();
    void closeEvent(QCloseEvent *event) override;

private:
    QSettings settings;
};

}  // namespace qnemu