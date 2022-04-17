/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtGui/QAction>
#include <QtGui/QKeySequence>
#include <QtGui/QRasterWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>

#include "qnemu/MainWindow.h"
#include "qnemu/display/RasterDisplay.h"
#include "qnemu/gb/Gb.h"

namespace qnemu
{

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QRect rect = settings.value("window/geometry", QRect(0, 0, 320, 288)).toRect();
    setGeometry(rect);

    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* fileAction = new QAction(tr("&Open..."), this);
    fileAction->setShortcut(QKeySequence::Open);
    fileAction->setStatusTip(tr("Open a ROM file"));
    connect(fileAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(fileAction);

    fileMenu->addSeparator();

    QAction* exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);
}

MainWindow::~MainWindow()
{
    if (gb) {
        gb->getDisplay()->setParent(nullptr);
    }
    setCentralWidget(nullptr);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ROM file"), "", tr("ROM files (*.gb *.gbc *.gbz *.gba *.sgb *.zip)"));
    if (fileName.isEmpty()) {
        return;
    }
    resize(320, 288 + menuBar()->height());

    if (!gb) {
        gb = std::make_unique<Gb>();
        QWidget* displayWidgets = QWidget::createWindowContainer(gb->getDisplay().get(), this);
        setCentralWidget(displayWidgets);
    }
    gb->loadCartridge(fileName.toStdString().c_str());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("window/geometry", geometry());
    QMainWindow::closeEvent(event);
}

}  // namespace qnemu