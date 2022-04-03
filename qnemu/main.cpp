/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <QtWidgets/QApplication>

#include "qnemu/DeviceInterface.h"
#include "qnemu/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("QShen");
    QApplication::setOrganizationDomain("blog.qshen.cc");
    QApplication::setApplicationName("QNEmu");
    QApplication::setApplicationVersion("0.1.0");
    
    qnemu::MainWindow window;
    window.show();

    return app.exec();
}