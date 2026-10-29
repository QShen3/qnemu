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

#include "qnemu/AppController.h"
#include "qnemu/EmulatorItem.h"
#include "qnemu/display/DisplayInterface.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

AppController::AppController(GbModel model, QObject* parent) :
    QObject(parent),
    model(model)
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
    if (showingGameValue) {
        showingGameValue = false;
        emit showingGameChanged();
    }
}

double AppController::scaleFactor() const
{
    return settingsValue->scalePercent() / 100.0;
}

void AppController::attachDisplay(QObject* item)
{
    displayItem = qobject_cast<EmulatorItem*>(item);
    applyKeyBindings();
}

void AppController::openRom(const QUrl& url)
{
    if (!displayItem || !url.isLocalFile()) {
        return;
    }

    const QString path = url.toLocalFile();
    gb.reset();
    emit hasGameChanged();

    try {
        auto display = std::shared_ptr<DisplayInterface>(displayItem,
            [](DisplayInterface*) {});
        auto input = std::shared_ptr<InputInterface>(displayItem,
            [](InputInterface*) {});

        auto sound = std::make_unique<DefaultSound>();
        auto system = std::make_unique<Gb>(model, display, std::move(sound), input);
        system->setSaveDirectory(settingsValue->savePath().toStdString());
        system->setSoundEnabled(settingsValue->soundEnabled());
        system->loadCartridge(path.toStdString().c_str());

        gb = std::move(system);
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
