/*
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>

#include "qnemu/DeviceInterface.h"
#include "qnemu/MainWindow.h"
#include "qnemu/gb/Gb.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("QShen");
    QApplication::setOrganizationDomain("blog.qshen.cc");
    QApplication::setApplicationName("QNEmu");
    QApplication::setApplicationVersion("0.1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("QNEmu Game Boy / Game Boy Color emulator");
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption modelOption(
        QStringList {"m", "model"},
        "Hardware model: cgb (default), dmg or sgb.",
        "model",
        "cgb");
    parser.addOption(modelOption);
    parser.process(app);

    const QString modelName = parser.value(modelOption).toLower();
    qnemu::GbModel model;
    if (modelName == "cgb") {
        model = qnemu::GbModel::cgb;
    } else if (modelName == "dmg") {
        model = qnemu::GbModel::dmg;
    } else if (modelName == "sgb") {
        model = qnemu::GbModel::sgb;
    } else {
        qCritical().noquote() << "Unknown model:" << modelName
                              << "(expected cgb, dmg or sgb)";
        return 1;
    }

    qnemu::MainWindow window(model);
    window.show();

    return app.exec();
}
