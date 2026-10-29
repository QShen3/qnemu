/*
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/qqml.h>
#include <QtQuickControls2/QQuickStyle>

#include "qnemu/AppController.h"
#include "qnemu/EmulatorItem.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QGuiApplication::setOrganizationName("QShen");
    QGuiApplication::setOrganizationDomain("blog.qshen.cc");
    QGuiApplication::setApplicationName("QNEmu");
    QGuiApplication::setApplicationVersion("0.1.0");

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
    const QCommandLineOption romOption(
        QStringList {"r", "rom"},
        "Load a ROM file on startup.",
        "path");
    parser.addOption(romOption);
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

    // Prefer the platform-native Quick Controls style.  The fallback keeps
    // Qt versions/platforms without a native style usable.
    QQuickStyle::setFallbackStyle(QStringLiteral("Fusion"));
#if defined(Q_OS_MACOS)
    QQuickStyle::setStyle(QStringLiteral("macOS"));
#elif defined(Q_OS_WIN)
    QQuickStyle::setStyle(QStringLiteral("FluentWinUI3"));
#elif defined(Q_OS_IOS)
    QQuickStyle::setStyle(QStringLiteral("iOS"));
#else
    QQuickStyle::setStyle(QStringLiteral("Fusion"));
#endif

    qmlRegisterType<qnemu::EmulatorItem>("QNEmu", 1, 0, "EmulatorItem");

    qnemu::AppController controller(model);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (parser.isSet(romOption)) {
        controller.openRom(QUrl::fromLocalFile(parser.value(romOption)));
    }

    const int result = app.exec();
    controller.shutdown();
    return result;
}
