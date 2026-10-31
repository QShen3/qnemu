/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <exception>
#include <memory>
#include <string>
#include <utility>

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QImage>

#include "qnemu/ui/AppController.h"
#include "qnemu/ui/EmulatorItem.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/display/DisplayInterface.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

AppController::AppController(std::optional<GbModel> cliModelOverride, QObject* parent) :
    QObject(parent),
    modelOverride(cliModelOverride)
{
    settingsValue = new AppSettings(this);
    libraryValue = new GameLibrary(this);
    libraryValue->setPaths(settingsValue->gbLibraryPath(), settingsValue->gbcLibraryPath());

    connect(settingsValue, &AppSettings::gbLibraryPathChanged, this, [this]() {
        libraryValue->setPaths(settingsValue->gbLibraryPath(), settingsValue->gbcLibraryPath());
    });
    connect(settingsValue, &AppSettings::gbcLibraryPathChanged, this, [this]() {
        libraryValue->setPaths(settingsValue->gbLibraryPath(), settingsValue->gbcLibraryPath());
    });
    connect(settingsValue, &AppSettings::scalePercentChanged, this, &AppController::scaleFactorChanged);
    connect(settingsValue, &AppSettings::soundEnabledChanged, this, [this]() {
        if (gb) {
            gb->setSoundEnabled(settingsValue->soundEnabled());
        }
    });
    connect(settingsValue, &AppSettings::gbLaunchModelChanged, this, [this]() {
        if (gb && !currentRomCgbOnly) {
            setStatusText(QStringLiteral("GB launch mode changed; reopen the game to apply."));
        }
    });
    connect(settingsValue, &AppSettings::loadCgbBootRomChanged, this, [this]() {
        if (gb) {
            setStatusText(QStringLiteral("CGB boot ROM setting changed; reopen the game to apply."));
        }
    });
    connect(settingsValue, &AppSettings::activeKeyBindingsChanged, this, &AppController::applyKeyBindings);
    connect(settingsValue, &AppSettings::activeKeyProfileChanged, this, &AppController::applyKeyBindings);
}

AppController::~AppController()
{
    shutdown();
}

void AppController::shutdown()
{
    gb.reset();
    displayItem = nullptr;
    currentRomPath.clear();
    currentRomCgbOnly = false;
    setFrameSize(160, 144);
    pausedValue = false;
    if (showingGameValue) {
        showingGameValue = false;
        emit showingGameChanged();
    }
}

double AppController::scaleFactor() const
{
    return settingsValue->scalePercent() / 100.0;
}

void AppController::setFrameSize(int width, int height)
{
    if (width == frameWidthValue && height == frameHeightValue) {
        return;
    }
    frameWidthValue = width;
    frameHeightValue = height;
    emit frameSizeChanged();
}

void AppController::attachDisplay(QObject* item)
{
    displayItem = qobject_cast<EmulatorItem*>(item);
    applyKeyBindings();
}

GbModel AppController::launchModel(bool cgbOnly) const
{
    // GBC-only cartridges can only run on CGB hardware, regardless of the
    // GB platform preference.
    if (cgbOnly) {
        return GbModel::cgb;
    }
    if (modelOverride) {
        return *modelOverride;
    }
    const QString model = settingsValue->gbLaunchModel();
    if (model == QStringLiteral("dmg")) {
        return GbModel::dmg;
    }
    if (model == QStringLiteral("sgb")) {
        return GbModel::sgb;
    }
    return GbModel::cgb;
}

void AppController::openRom(const QUrl& url)
{
    if (!displayItem || !url.isLocalFile()) {
        return;
    }

    const QString path = url.toLocalFile();
    if (pausedValue) {
        pausedValue = false;
        emit pausedChanged();
    }
    gb.reset();
    emit hasGameChanged();

    try {
        auto display = std::shared_ptr<DisplayInterface>(displayItem,
            [](DisplayInterface*) {});
        auto input = std::shared_ptr<InputInterface>(displayItem,
            [](InputInterface*) {});

        const GbCartridgeHeader header = GbCartridge::readHeader(path.toStdString());
        currentRomCgbOnly = header.cgbOnly;
        const GbModel targetModel = launchModel(currentRomCgbOnly);

        auto sound = std::make_unique<DefaultSound>();
        auto system = std::make_unique<Gb>(targetModel, display,
            std::move(sound), input);
        system->setSaveDirectory(settingsValue->savePath().toStdString());
        system->setSoundEnabled(settingsValue->soundEnabled());
        system->setBootRomEnabled(settingsValue->loadCgbBootRom());
        system->loadCartridge(path.toStdString().c_str());

        gb = std::move(system);
        setFrameSize(targetModel == GbModel::sgb ? 256 : 160,
            targetModel == GbModel::sgb ? 224 : 144);
        currentRomPath = path;
        libraryValue->addOpenedGame(path);

        setWindowTitle(QFileInfo(path).fileName() + QStringLiteral(" - QNEmu"));
        setStatusText(QStringLiteral("Loaded ") + QFileInfo(path).fileName());
        if (!showingGameValue) {
            showingGameValue = true;
            emit showingGameChanged();
        }
        emit hasGameChanged();
    } catch (const std::exception& error) {
        qWarning().noquote() << "Failed to load ROM:" << error.what();
        currentRomCgbOnly = false;
        setFrameSize(160, 144);
        setWindowTitle(QStringLiteral("QNEmu"));
        setStatusText(QStringLiteral("Failed to load ROM: ") + error.what());
        if (showingGameValue) {
            showingGameValue = false;
            emit showingGameChanged();
        }
    }
}

void AppController::openGame(const QString& path)
{
    openRom(QUrl::fromLocalFile(path));
}

void AppController::addGame(const QUrl& url)
{
    if (!url.isLocalFile()) {
        return;
    }
    libraryValue->addGame(url.toLocalFile());
    setStatusText(QStringLiteral("Added to library: ") + QFileInfo(url.toLocalFile()).fileName());
}

void AppController::removeGame(const QString& path)
{
    libraryValue->removeGame(path);
    setStatusText(QStringLiteral("Removed from library: ") + QFileInfo(path).fileName());
}

void AppController::refreshLibrary()
{
    libraryValue->refresh();
    setStatusText(QStringLiteral("Library refreshed"));
}

void AppController::showLibrary()
{
    if (!showingGameValue) {
        return;
    }
    showingGameValue = false;
    const bool hadGame = gb != nullptr;
    setPaused(true);
    if (hadGame) {
        setStatusText(QStringLiteral("Paused"));
    }
    emit showingGameChanged();
}

void AppController::showGame()
{
    if (!gb || showingGameValue) {
        return;
    }
    showingGameValue = true;
    emit showingGameChanged();
}

void AppController::togglePause()
{
    setPaused(!pausedValue);
}

void AppController::pause()
{
    setPaused(true);
}

void AppController::resume()
{
    setPaused(false);
}

void AppController::setPaused(bool value)
{
    if (!gb || value == pausedValue) {
        return;
    }
    pausedValue = value;
    gb->setPaused(value);
    emit pausedChanged();
}

QString AppController::localPath(const QUrl& url) const
{
    return url.toLocalFile();
}

bool AppController::takeScreenshot()
{    if (!gb) {
        return false;
    }

    const QString directory = settingsValue->screenshotDirectory();
    QDir().mkpath(directory);
    const QString format = settingsValue->screenshotFormat();
    const QString baseName = currentRomPath.isEmpty()
        ? QStringLiteral("screenshot")
        : QFileInfo(currentRomPath).completeBaseName();
    const QString fileName = baseName + QLatin1Char('-')
        + QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"))
        + QLatin1Char('.') + format;
    const QString path = QDir(directory).filePath(fileName);

    const QImage image = displayItem->snapshot();
    if (image.isNull()) {
        setStatusText(QStringLiteral("No frame available for screenshot"));
        return false;
    }

    const QByteArray formatBytes = format.toUpper().toLatin1();
    const bool saved = image.save(path, formatBytes.constData());
    setStatusText(saved
        ? QStringLiteral("Screenshot saved: ") + path
        : QStringLiteral("Failed to save screenshot"));
    return saved;
}

void AppController::applyKeyBindings()
{
    if (displayItem) {
        displayItem->setKeyBindings(settingsValue->activeKeyBindings());
    }
}

void AppController::setWindowTitle(const QString& newTitle)
{
    if (title == newTitle) {
        return;
    }
    title = newTitle;
    emit windowTitleChanged();
}

void AppController::setStatusText(const QString& newStatus)
{
    if (status == newStatus) {
        return;
    }
    status = newStatus;
    emit statusTextChanged();
}

}  // namespace qnemu
