/*
 *  Copyright [2020] <qazxdrcssc2006@163.com>
 */

#include <optional>

#include <QtCore/QCommandLineOption>
#include <QtCore/QCommandLineParser>
#include <QtCore/QDebug>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QVariantList>
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
    // Do not call this "--platform": QGuiApplication treats "-platform" as
    // its own Qt platform-plugin option and would consume the argument.
    const QCommandLineOption platformOption(
        QStringList {"p", "launch-platform"},
        "Override the launch platform for this run: gb, sgb or gbc.",
        "platform");
    parser.addOption(platformOption);
    const QCommandLineOption modelOption(
        QStringList {"m", "model"},
        "Deprecated alias for --launch-platform: dmg, sgb or cgb.",
        "model");
    parser.addOption(modelOption);
    const QCommandLineOption romOption(
        QStringList {"r", "rom"},
        "Load a ROM file on startup.",
        "path");
    parser.addOption(romOption);
    const QCommandLineOption linkRomOption(
        QStringList {"link-rom"},
        "Load another ROM for link play; repeat up to three times. Requires --rom.",
        "path");
    parser.addOption(linkRomOption);
    parser.process(app);

    std::optional<qnemu::GbPlatform> platformOverride;
    if (parser.isSet(platformOption)) {
        const QString platformName = parser.value(platformOption).toLower();
        if (platformName == "gb") {
            platformOverride = qnemu::GbPlatform::gb;
        } else if (platformName == "sgb") {
            platformOverride = qnemu::GbPlatform::sgb;
        } else if (platformName == "gbc") {
            platformOverride = qnemu::GbPlatform::gbc;
        } else {
            qCritical().noquote() << "Unknown platform:" << platformName
                                  << "(expected gb, sgb or gbc)";
            return 1;
        }
    } else if (parser.isSet(modelOption)) {
        const QString modelName = parser.value(modelOption).toLower();
        if (modelName == "cgb" || modelName == "gbc") {
            platformOverride = qnemu::GbPlatform::gbc;
        } else if (modelName == "dmg" || modelName == "gb") {
            platformOverride = qnemu::GbPlatform::gb;
        } else if (modelName == "sgb") {
            platformOverride = qnemu::GbPlatform::sgb;
        } else {
            qCritical().noquote() << "Unknown model:" << modelName
                                  << "(expected dmg, sgb or cgb)";
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

    qnemu::AppController controller(platformOverride);
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("appController"), &controller);

    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (parser.isSet(linkRomOption)) {
        if (!parser.isSet(romOption)) {
            qCritical().noquote() << "--link-rom requires --rom";
            return 1;
        }

        QStringList paths;
        paths << parser.value(romOption);
        paths << parser.values(linkRomOption);
        if (paths.size() > 4) {
            qCritical().noquote() << "Link play supports at most 4 players";
            return 1;
        }

        if (paths.size() == 2) {
            controller.openLinkGame(QUrl::fromLocalFile(paths.at(0)),
                QUrl::fromLocalFile(paths.at(1)));
        } else {
            QVariantList saveSlots;
            QSet<QString> usedSaves;
            for (const QString& path : paths) {
                int chosen = -1;
                for (int slot = 0; slot < 4; ++slot) {
                    const QString save = controller.linkSavePath(path, slot);
                    if (!usedSaves.contains(save)) {
                        chosen = slot;
                        break;
                    }
                }
                if (chosen < 0) {
                    qCritical().noquote() << "Could not choose a save slot for" << path;
                    return 1;
                }
                saveSlots.append(chosen);
                usedSaves.insert(controller.linkSavePath(path, chosen));
            }
            controller.startLinkGame(paths, saveSlots);
        }
    } else if (parser.isSet(romOption)) {
        controller.openRom(QUrl::fromLocalFile(parser.value(romOption)));
    }

    const int result = app.exec();
    controller.shutdown();
    return result;
}
