/* 
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <QtWidgets/QApplication>

#include "qnemu/DeviceInterface.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("QShen");
    QApplication::setOrganizationDomain("blog.qshen.cc");
    QApplication::setApplicationName("QNemu");

    return app.exec();
}