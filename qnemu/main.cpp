/*
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <optional>

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

#include "qnemu/ui/AppController.h"
#include "qnemu/ui/EmulatorItem.h"

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
        "Override the hardware model for this run: cgb, dmg or sgb.",
        "model");
    parser.addOption(modelOption);
    const QCommandLineOption romOption(
        QStringList {"r", "rom"},
        "Load a ROM file on startup.",
        "path");
    parser.addOption(romOption);
    parser.process(app);

    std::optional<qnemu::GbModel> modelOverride;
    if (parser.isSet(modelOption)) {
        const QString modelName = parser.value(modelOption).toLower();
        if (modelName == "cgb") {
            modelOverride = qnemu::GbModel::cgb;
        } else if (modelName == "dmg") {
            modelOverride = qnemu::GbModel::dmg;
        } else if (modelName == "sgb") {
            modelOverride = qnemu::GbModel::sgb;
        } else {
            qCritical().noquote() << "Unknown model:" << modelName
                                  << "(expected cgb, dmg or sgb)";
            return 1;
        }
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
    qmlRegisterUncreatableType<qnemu::AppSettings>("QNEmu", 1, 0, "AppSettings",
        QStringLiteral("AppSettings is owned by AppController"));
    qmlRegisterUncreatableType<qnemu::GameLibrary>("QNEmu", 1, 0, "GameLibrary",
        QStringLiteral("GameLibrary is owned by AppController"));

    qnemu::AppController controller(modelOverride);
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
