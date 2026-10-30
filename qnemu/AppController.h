/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>
#include <optional>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "qnemu/AppSettings.h"
#include "qnemu/GameLibrary.h"
#include "qnemu/gb/Gb.h"

namespace qnemu
{

class EmulatorItem;

/**
 * C++ side of the QML shell.
 *
 * It owns the Gb system, settings and game library, and exposes only small
 * properties and invokable actions to QML.
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool hasGame READ hasGame NOTIFY hasGameChanged)
    Q_PROPERTY(bool showingGame READ showingGame NOTIFY showingGameChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(double scaleFactor READ scaleFactor NOTIFY scaleFactorChanged)
    Q_PROPERTY(AppSettings* settings READ settings CONSTANT)
    Q_PROPERTY(GameLibrary* library READ library CONSTANT)

public:
    explicit AppController(std::optional<GbModel> cliModelOverride, QObject* parent = nullptr);
    ~AppController() override;

    QString windowTitle() const { return title; }
    bool hasGame() const { return gb != nullptr; }
    bool showingGame() const { return showingGameValue; }
    QString statusText() const { return status; }
    double scaleFactor() const;
    AppSettings* settings() { return settingsValue; }
    GameLibrary* library() { return libraryValue; }

    Q_INVOKABLE void attachDisplay(QObject* item);
    Q_INVOKABLE void openRom(const QUrl& url);
    Q_INVOKABLE void openGame(const QString& path);
    Q_INVOKABLE void addGame(const QUrl& url);
    Q_INVOKABLE void removeGame(const QString& path);
    Q_INVOKABLE void refreshLibrary();
    Q_INVOKABLE void showLibrary();
    Q_INVOKABLE void showGame();
    Q_INVOKABLE bool takeScreenshot();
    Q_INVOKABLE QString localPath(const QUrl& url) const;

    /// Stops emulation before the QML scene and its display item are destroyed.
    void shutdown();

signals:
    void windowTitleChanged();
    void hasGameChanged();
    void showingGameChanged();
    void statusTextChanged();
    void scaleFactorChanged();

private:
    void setWindowTitle(const QString& newTitle);
    void setStatusText(const QString& newStatus);
    void applyKeyBindings();
    GbModel launchModel() const;

    std::optional<GbModel> modelOverride;
    AppSettings* settingsValue = nullptr;
    GameLibrary* libraryValue = nullptr;
    EmulatorItem* displayItem = nullptr;
    std::unique_ptr<Gb> gb;
    QString title = QStringLiteral("QNEmu");
    QString status;
    QString currentRomPath;
    bool showingGameValue = false;
};

}  // namespace qnemu
