/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <array>
#include <memory>
#include <optional>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QVariantList>

#include "qnemu/ui/AppSettings.h"
#include "qnemu/ui/GameLibrary.h"
#include "qnemu/gb/Gb.h"
#include "qnemu/gb/cartridge/GbCartridgeHeader.h"
#include "qnemu/gb/serial/GbSerialLink.h"

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
    Q_PROPERTY(int frameWidth READ frameWidth NOTIFY frameSizeChanged)
    Q_PROPERTY(int frameHeight READ frameHeight NOTIFY frameSizeChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)
    Q_PROPERTY(QString currentGamePath READ currentGamePath
        NOTIFY currentGamePathChanged)
    Q_PROPERTY(bool linkActive READ linkActive NOTIFY linkActiveChanged)
    Q_PROPERTY(bool linkPaused READ linkPaused NOTIFY linkPausedChanged)
    Q_PROPERTY(QString linkTitle1 READ linkTitle1 NOTIFY linkTitlesChanged)
    Q_PROPERTY(QString linkTitle2 READ linkTitle2 NOTIFY linkTitlesChanged)
    Q_PROPERTY(AppSettings* settings READ settings CONSTANT)
    Q_PROPERTY(GameLibrary* library READ library CONSTANT)

public:
    explicit AppController(std::optional<GbPlatform> cliPlatformOverride, QObject* parent = nullptr);
    ~AppController() override;

    QString windowTitle() const { return title; }
    bool hasGame() const { return gb != nullptr; }
    bool showingGame() const { return showingGameValue; }
    QString statusText() const { return status; }
    double scaleFactor() const;
    int frameWidth() const { return frameWidthValue; }
    int frameHeight() const { return frameHeightValue; }
    bool paused() const { return pausedValue; }
    QString currentGamePath() const { return currentRomPath; }
    bool linkActive() const { return linkActiveValue; }
    bool linkPaused() const { return linkPausedValue; }
    QString linkTitle1() const { return linkTitles.at(0); }
    QString linkTitle2() const { return linkTitles.at(1); }
    AppSettings* settings() { return settingsValue; }
    GameLibrary* library() { return libraryValue; }

    Q_INVOKABLE void attachDisplay(QObject* item);
    Q_INVOKABLE void attachLinkDisplay(int slot, QObject* item);
    Q_INVOKABLE void openRom(const QUrl& url);
    Q_INVOKABLE void openLinkGame(const QUrl& first, const QUrl& second);
    Q_INVOKABLE void startLinkGame(const QString& firstPath, const QString& secondPath,
        int firstSaveSlot, int secondSaveSlot);
    Q_INVOKABLE QVariantList saveSlots(const QString& romPath) const;
    Q_INVOKABLE QString saveDataBlockedReason(const QString& romPath) const;
    Q_INVOKABLE bool copySave(const QString& romPath, int fromSlot, int toSlot);
    Q_INVOKABLE bool deleteSave(const QString& romPath, int slot);
    Q_INVOKABLE bool backupSave(const QString& romPath, int slot);
    Q_INVOKABLE bool importSave(const QString& romPath, int slot, const QUrl& source);
    Q_INVOKABLE bool exportSave(const QString& romPath, int slot, const QUrl& target);
    Q_INVOKABLE void openGameWithSave(const QString& path, int slot);
    Q_INVOKABLE void closeGame();
    Q_INVOKABLE QString linkSavePath(const QString& romPath, int slot) const;
    Q_INVOKABLE void stopLink();
    Q_INVOKABLE void toggleLinkPause();
    Q_INVOKABLE void openGame(const QString& path);
    Q_INVOKABLE void addGame(const QUrl& url);
    Q_INVOKABLE void removeGame(const QString& path);
    Q_INVOKABLE void refreshLibrary();
    Q_INVOKABLE void showLibrary();
    Q_INVOKABLE void showGame();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
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
    void frameSizeChanged();
    void pausedChanged();
    void currentGamePathChanged();
    void linkActiveChanged();
    void linkPausedChanged();
    void linkTitlesChanged();

private:
    struct LoadedGame
    {
        std::unique_ptr<Gb> system;
        QString displayName;
        bool cgbOnly = false;
    };

    void setWindowTitle(const QString& newTitle);
    void setStatusText(const QString& newStatus);
    void setPaused(bool value);
    void setCurrentRomPath(const QString& path);
    void setFrameSize(int width, int height);
    void applyKeyBindings();
    GbPlatform launchPlatform(const GbCartridgeHeader& header) const;
    LoadedGame loadGame(const QString& path, EmulatorItem* item,
        const QString& saveDirectory,
        std::shared_ptr<GbSerialLink> serialLink = nullptr);
    void openGamePath(const QString& path, int slot);
    void closeCurrentGame();
    QString saveDirectoryForSlot(int slot) const;
    QString saveSlotLabel(int slot) const;
    QString saveSlotSuffix(int slot) const;
    QString rtcFilePathForSlot(const QString& romPath, int slot) const;
    bool saveDataEditable(const QString& romPath) const;

    std::optional<GbPlatform> platformOverride;
    AppSettings* settingsValue = nullptr;
    GameLibrary* libraryValue = nullptr;
    EmulatorItem* displayItem = nullptr;
    std::unique_ptr<Gb> gb;
    std::array<EmulatorItem*, 2> linkDisplayItems {};
    std::array<std::unique_ptr<Gb>, 2> linkSystems;
    std::array<QString, 2> linkTitles;
    bool linkActiveValue = false;
    bool linkPausedValue = false;
    QString title = QStringLiteral("QNEmu");
    QString status;
    QString currentRomPath;
    int currentSaveSlotValue = 0;
    bool currentRomCgbOnly = false;
    int frameWidthValue = 160;
    int frameHeightValue = 144;
    bool showingGameValue = false;
    bool pausedValue = false;
};

}  // namespace qnemu
