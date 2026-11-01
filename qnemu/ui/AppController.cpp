/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QVariantMap>
#include <QtGui/QImage>

#include "qnemu/ui/AppController.h"
#include "qnemu/ui/EmulatorItem.h"
#include "qnemu/ui/LaunchPolicy.h"
#include "qnemu/ui/RomArchive.h"
#include "qnemu/ui/SavePaths.h"
#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/serial/GbLocalSerialLink.h"
#include "qnemu/display/DisplayInterface.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

namespace
{

bool sameFile(const QString& left, const QString& right)
{
    if (left.isEmpty() || right.isEmpty()) {
        return false;
    }
    const QFileInfo leftInfo(left);
    const QFileInfo rightInfo(right);
    const QString leftPath = leftInfo.canonicalFilePath().isEmpty()
        ? leftInfo.absoluteFilePath()
        : leftInfo.canonicalFilePath();
    const QString rightPath = rightInfo.canonicalFilePath().isEmpty()
        ? rightInfo.absoluteFilePath()
        : rightInfo.canonicalFilePath();
    return leftPath == rightPath;
}

}  // namespace

AppController::AppController(std::optional<GbPlatform> cliPlatformOverride, QObject* parent) :
    QObject(parent),
    platformOverride(cliPlatformOverride)
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
    const auto launchPlatformChanged = [this]() {
        if (gb && !currentRomCgbOnly) {
            setStatusText(QStringLiteral("Launch platform changed; reopen the game to apply."));
        }
    };
    connect(settingsValue, &AppSettings::dmgOnlyLaunchPlatformChanged,
        this, launchPlatformChanged);
    connect(settingsValue, &AppSettings::dualModeLaunchPlatformChanged,
        this, launchPlatformChanged);
    connect(settingsValue, &AppSettings::activeKeyBindingsChanged, this, &AppController::applyKeyBindings);
    connect(settingsValue, &AppSettings::activeKeyProfileChanged, this, &AppController::applyKeyBindings);
}

AppController::~AppController()
{
    shutdown();
}

void AppController::shutdown()
{
    stopLink();
    gb.reset();
    displayItem = nullptr;
    linkDisplayItems = {};
    currentSaveSlotValue = 0;
    setCurrentRomPath(QString());
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

void AppController::attachLinkDisplay(int slot, QObject* item)
{
    if (slot < 0 || slot > 1) {
        return;
    }
    linkDisplayItems.at(slot) = qobject_cast<EmulatorItem*>(item);
    applyKeyBindings();
}

GbPlatform AppController::launchPlatform(const GbCartridgeHeader& header) const
{
    const GbPlatform dmgOnly = launchPlatformFromString(
        settingsValue->dmgOnlyLaunchPlatform().toStdString())
                                   .value_or(GbPlatform::gb);
    const GbPlatform dualMode = launchPlatformFromString(
        settingsValue->dualModeLaunchPlatform().toStdString())
                                     .value_or(GbPlatform::gbc);
    return selectLaunchPlatform(header, dmgOnly, dualMode, platformOverride);
}

void AppController::openRom(const QUrl& url)
{
    if (!displayItem || !url.isLocalFile()) {
        return;
    }
    openGamePath(url.toLocalFile(), 0);
}

void AppController::openGameWithSave(const QString& path, int slot)
{
    if (!displayItem || path.isEmpty() || slot < 0 || slot > 2) {
        return;
    }
    if (gb && sameFile(path, currentRomPath) && currentSaveSlotValue == slot) {
        resume();
        showGame();
        return;
    }
    openGamePath(path, slot);
}

void AppController::openGamePath(const QString& path, int slot)
{
    if (!displayItem || path.isEmpty() || slot < 0 || slot > 2) {
        return;
    }

    stopLink();
    closeCurrentGame();

    try {
        auto loaded = loadGame(path, displayItem, saveDirectoryForSlot(slot));
        gb = std::move(loaded.system);
        currentSaveSlotValue = slot;
        currentRomCgbOnly = loaded.cgbOnly;
        const auto [frameWidth, frameHeight] = gb->frameSize();
        setFrameSize(frameWidth, frameHeight);
        setCurrentRomPath(path);
        libraryValue->addOpenedGame(path);

        QString title = loaded.displayName;
        if (slot != 0) {
            title += QStringLiteral(" - ") + saveSlotLabel(slot);
        }
        setWindowTitle(title + QStringLiteral(" - QNEmu"));
        setStatusText(tr("Loaded %1 (%2)")
            .arg(loaded.displayName, saveSlotLabel(slot)));
        showGame();
        emit hasGameChanged();
    } catch (const std::exception& error) {
        qWarning().noquote() << "Failed to load ROM:" << error.what();
        currentSaveSlotValue = 0;
        currentRomCgbOnly = false;
        setCurrentRomPath(QString());
        setFrameSize(160, 144);
        setWindowTitle(QStringLiteral("QNEmu"));
        setStatusText(QStringLiteral("Failed to load ROM: ") + error.what());
        if (showingGameValue) {
            showingGameValue = false;
            emit showingGameChanged();
        }
    }
}

void AppController::closeGame()
{
    if (!gb) {
        return;
    }
    closeCurrentGame();
    setStatusText(tr("Game closed; save data flushed"));
}

void AppController::openLinkGame(const QUrl& first, const QUrl& second)
{
    if (!first.isLocalFile() || !second.isLocalFile()) {
        return;
    }

    const QString firstPath = first.toLocalFile();
    const QString secondPath = second.toLocalFile();
    // The simple entry point (CLI) keeps normal saves for both players.  Two
    // copies of the same ROM would resolve to one file, so give the second
    // player its own persistent link save.
    const int secondSlot =
        linkSavePath(firstPath, 0) == linkSavePath(secondPath, 0) ? 2 : 0;
    startLinkGame(firstPath, secondPath, 0, secondSlot);
}

void AppController::startLinkGame(const QString& firstPath, const QString& secondPath,
    int firstSaveSlot, int secondSaveSlot)
{
    if (!linkDisplayItems.at(0) || !linkDisplayItems.at(1)
        || firstPath.isEmpty() || secondPath.isEmpty()) {
        return;
    }
    if (firstSaveSlot < 0 || firstSaveSlot > 2
        || secondSaveSlot < 0 || secondSaveSlot > 2) {
        return;
    }

    const QString firstSave = linkSavePath(firstPath, firstSaveSlot);
    const QString secondSave = linkSavePath(secondPath, secondSaveSlot);
    if (firstSave == secondSave) {
        setStatusText(tr("Link Play needs two different save files"));
        return;
    }

    stopLink();
    closeCurrentGame();

    try {
        const auto [firstLink, secondLink] = createLocalSerialLinkPair();
        auto firstGame = loadGame(firstPath, linkDisplayItems.at(0),
            saveDirectoryForSlot(firstSaveSlot), firstLink);
        auto secondGame = loadGame(secondPath, linkDisplayItems.at(1),
            saveDirectoryForSlot(secondSaveSlot), secondLink);

        linkSystems.at(0) = std::move(firstGame.system);
        linkSystems.at(1) = std::move(secondGame.system);
        linkTitles.at(0) = firstGame.displayName;
        linkTitles.at(1) = secondGame.displayName;
        linkActiveValue = true;
        linkPausedValue = false;
        emit linkTitlesChanged();
        emit linkPausedChanged();
        emit linkActiveChanged();

        libraryValue->addOpenedGame(firstPath);
        libraryValue->addOpenedGame(secondPath);
        setStatusText(tr("Link play started (%1 / %2)")
            .arg(saveSlotLabel(firstSaveSlot), saveSlotLabel(secondSaveSlot)));
    } catch (const std::exception& error) {
        stopLink();
        qWarning().noquote() << "Failed to start link play:" << error.what();
        setStatusText(QStringLiteral("Failed to start link play: ") + error.what());
    }
}

QVariantList AppController::saveSlots(const QString& romPath) const
{
    QVariantList options;
    for (int slot = 0; slot < 3; ++slot) {
        const QString path = linkSavePath(romPath, slot);
        const QString rtcPath = rtcFilePathForSlot(romPath, slot);
        const QFileInfo info(path);
        const QFileInfo rtcInfo(rtcPath);
        QVariantMap option;
        option.insert(QStringLiteral("slot"), slot);
        option.insert(QStringLiteral("label"), saveSlotLabel(slot));
        option.insert(QStringLiteral("path"), path);
        option.insert(QStringLiteral("exists"), info.exists());
        option.insert(QStringLiteral("size"), info.exists() ? info.size() : 0);
        option.insert(QStringLiteral("modified"),
            info.exists() ? info.lastModified().toString(Qt::ISODate) : QString());
        option.insert(QStringLiteral("rtcPath"), rtcPath);
        option.insert(QStringLiteral("rtcExists"), rtcInfo.exists());
        options.append(option);
    }
    return options;
}

QString AppController::saveDataBlockedReason(const QString& romPath) const
{
    if (linkActiveValue) {
        return QStringLiteral("link");
    }
    if (gb && sameFile(romPath, currentRomPath)) {
        return QStringLiteral("game");
    }
    return {};
}

bool AppController::saveDataEditable(const QString& romPath) const
{
    return saveDataBlockedReason(romPath).isEmpty();
}

QString AppController::rtcFilePathForSlot(const QString& romPath, int slot) const
{
    if (slot < 0 || slot > 2) {
        return {};
    }
    return rtcPathForRom(saveDirectoryForSlot(slot), romPath);
}

QString AppController::saveSlotSuffix(int slot) const
{
    switch (slot) {
    case 0:
        return QStringLiteral("game");
    case 1:
        return QStringLiteral("link1");
    default:
        return QStringLiteral("link2");
    }
}

bool AppController::copySave(const QString& romPath, int fromSlot, int toSlot)
{
    if (!saveDataEditable(romPath)) {
        setStatusText(tr("Close the running game or link play before changing saves"));
        return false;
    }
    if (fromSlot < 0 || fromSlot > 2 || toSlot < 0 || toSlot > 2
        || fromSlot == toSlot) {
        return false;
    }

    QString error;
    if (!copySaveFiles(linkSavePath(romPath, fromSlot),
            rtcFilePathForSlot(romPath, fromSlot),
            linkSavePath(romPath, toSlot),
            rtcFilePathForSlot(romPath, toSlot), error)) {
        setStatusText(tr("Copy failed: %1").arg(error));
        return false;
    }
    setStatusText(tr("Copied %1 to %2")
        .arg(saveSlotLabel(fromSlot), saveSlotLabel(toSlot)));
    return true;
}

bool AppController::deleteSave(const QString& romPath, int slot)
{
    if (!saveDataEditable(romPath)) {
        setStatusText(tr("Close the running game or link play before changing saves"));
        return false;
    }
    if (slot < 0 || slot > 2) {
        return false;
    }

    bool removed = false;
    for (const QString& path : {linkSavePath(romPath, slot),
             rtcFilePathForSlot(romPath, slot)}) {
        if (!QFileInfo::exists(path)) {
            continue;
        }
        QFile file(path);
        if (!file.remove()) {
            setStatusText(tr("Delete failed: %1").arg(file.errorString()));
            return false;
        }
        removed = true;
    }
    if (!removed) {
        setStatusText(tr("No save data to delete"));
        return false;
    }
    setStatusText(tr("Deleted %1").arg(saveSlotLabel(slot)));
    return true;
}

bool AppController::backupSave(const QString& romPath, int slot)
{
    if (!saveDataEditable(romPath)) {
        setStatusText(tr("Close the running game or link play before changing saves"));
        return false;
    }
    if (slot < 0 || slot > 2) {
        return false;
    }

    const QString sourceSav = linkSavePath(romPath, slot);
    if (!QFileInfo::exists(sourceSav)) {
        setStatusText(tr("No save data to back up"));
        return false;
    }

    const QString backupDirectory =
        QDir(settingsValue->savePath()).filePath(QStringLiteral("backups"));
    const QString timestamp = QDateTime::currentDateTime()
        .toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    const QString backupSav = QDir(backupDirectory).filePath(
        QStringLiteral("%1-%2-%3.sav")
            .arg(saveFileStem(romPath), saveSlotSuffix(slot), timestamp));
    const QFileInfo backupInfo(backupSav);
    const QString backupRtc = backupInfo.absolutePath() + QLatin1Char('/')
        + backupInfo.completeBaseName() + QStringLiteral(".rtc");

    QString error;
    if (!copySaveFiles(sourceSav, rtcFilePathForSlot(romPath, slot),
            backupSav, backupRtc, error)) {
        setStatusText(tr("Backup failed: %1").arg(error));
        return false;
    }
    setStatusText(tr("Backup saved: %1").arg(backupSav));
    return true;
}

bool AppController::importSave(const QString& romPath, int slot, const QUrl& source)
{
    if (!saveDataEditable(romPath)) {
        setStatusText(tr("Close the running game or link play before changing saves"));
        return false;
    }
    if (slot < 0 || slot > 2 || !source.isLocalFile()) {
        return false;
    }

    const QString sourceSav = source.toLocalFile();
    if (!QFileInfo::exists(sourceSav)) {
        setStatusText(tr("Import failed: the selected file does not exist"));
        return false;
    }
    const QFileInfo sourceInfo(sourceSav);
    const QString sourceRtc = sourceInfo.absolutePath() + QLatin1Char('/')
        + sourceInfo.completeBaseName() + QStringLiteral(".rtc");

    QString error;
    if (!copySaveFiles(sourceSav, sourceRtc, linkSavePath(romPath, slot),
            rtcFilePathForSlot(romPath, slot), error)) {
        setStatusText(tr("Import failed: %1").arg(error));
        return false;
    }
    setStatusText(tr("Imported %1 into %2")
        .arg(sourceInfo.fileName(), saveSlotLabel(slot)));
    return true;
}

bool AppController::exportSave(const QString& romPath, int slot, const QUrl& target)
{
    if (!saveDataEditable(romPath)) {
        setStatusText(tr("Close the running game or link play before changing saves"));
        return false;
    }
    if (slot < 0 || slot > 2 || !target.isLocalFile()) {
        return false;
    }

    QString targetSav = target.toLocalFile();
    if (targetSav.isEmpty()) {
        return false;
    }
    if (QFileInfo(targetSav).suffix().isEmpty()) {
        targetSav += QStringLiteral(".sav");
    }
    const QFileInfo targetInfo(targetSav);
    const QString targetRtc = targetInfo.absolutePath() + QLatin1Char('/')
        + targetInfo.completeBaseName() + QStringLiteral(".rtc");

    const QString sourceSav = linkSavePath(romPath, slot);
    if (!QFileInfo::exists(sourceSav)) {
        setStatusText(tr("No save data to export"));
        return false;
    }

    QString error;
    if (!copySaveFiles(sourceSav, rtcFilePathForSlot(romPath, slot),
            targetSav, targetRtc, error)) {
        setStatusText(tr("Export failed: %1").arg(error));
        return false;
    }
    setStatusText(tr("Exported %1 to %2").arg(saveSlotLabel(slot), targetSav));
    return true;
}

QString AppController::linkSavePath(const QString& romPath, int slot) const
{
    if (slot < 0 || slot > 2) {
        return {};
    }
    return savePathForRom(saveDirectoryForSlot(slot), romPath);
}

void AppController::stopLink()
{
    const bool wasActive = linkActiveValue;
    const bool hadTitles = !linkTitles.at(0).isEmpty() || !linkTitles.at(1).isEmpty();

    linkSystems.at(0).reset();
    linkSystems.at(1).reset();
    linkTitles.at(0).clear();
    linkTitles.at(1).clear();
    linkActiveValue = false;

    if (linkPausedValue) {
        linkPausedValue = false;
        emit linkPausedChanged();
    }
    if (wasActive) {
        emit linkActiveChanged();
    }
    if (hadTitles) {
        emit linkTitlesChanged();
    }
}

void AppController::toggleLinkPause()
{
    if (!linkActiveValue) {
        return;
    }

    linkPausedValue = !linkPausedValue;
    for (auto& system : linkSystems) {
        if (system) {
            system->setPaused(linkPausedValue);
        }
    }
    emit linkPausedChanged();
}

AppController::LoadedGame AppController::loadGame(const QString& path, EmulatorItem* item,
    const QString& saveDirectory, std::shared_ptr<GbSerialLink> serialLink)
{
    if (!item) {
        throw std::runtime_error("No display attached");
    }

    const auto display = std::shared_ptr<DisplayInterface>(item,
        [](DisplayInterface*) {});
    const auto input = std::shared_ptr<InputInterface>(item,
        [](InputInterface*) {});

    const bool archived = RomArchive::isArchivePath(path);
    GbCartridgeHeader header;
    std::vector<uint8_t> romData;
    QString displayName = QFileInfo(path).fileName();
    if (archived) {
        RomArchiveEntry archive;
        QString error;
        if (!RomArchive::read(path, &archive, &error)) {
            throw std::runtime_error(error.toStdString());
        }
        romData.assign(archive.data.begin(), archive.data.end());
        header = GbCartridge::readHeader(romData);
        displayName += QStringLiteral(" / ") + archive.name;
    } else {
        header = GbCartridge::readHeader(path.toStdString());
    }

    const GbPlatform targetPlatform = launchPlatform(header);

    auto sound = std::make_unique<DefaultSound>();
    auto system = std::make_unique<Gb>(targetPlatform, display,
        std::move(sound), input, std::move(serialLink));
    system->setSaveDirectory(saveDirectory.toStdString());
    system->setSoundEnabled(settingsValue->soundEnabled());
    if (archived) {
        system->loadCartridge(romData, path.toStdString());
    } else {
        system->loadCartridge(path.toStdString().c_str());
    }

    return {std::move(system), displayName, header.cgbOnly};
}

void AppController::closeCurrentGame()
{
    if (!gb) {
        return;
    }

    gb.reset();
    currentSaveSlotValue = 0;
    currentRomCgbOnly = false;
    setCurrentRomPath(QString());
    setFrameSize(160, 144);
    pausedValue = false;
    emit pausedChanged();
    emit hasGameChanged();
    if (showingGameValue) {
        showingGameValue = false;
        emit showingGameChanged();
    }
}

QString AppController::saveDirectoryForSlot(int slot) const
{
    if (slot <= 0) {
        return settingsValue->savePath();
    }
    return QDir(settingsValue->savePath()).filePath(
        QStringLiteral("link%1").arg(slot));
}

QString AppController::saveSlotLabel(int slot) const
{
    switch (slot) {
    case 0:
        return tr("Game save");
    case 1:
        return tr("Link save 1");
    default:
        return tr("Link save 2");
    }
}

void AppController::openGame(const QString& path)
{
    if (gb && sameFile(path, currentRomPath)) {
        const bool wasPaused = pausedValue;
        if (wasPaused) {
            resume();
            setStatusText(QStringLiteral("Resumed ")
                + QFileInfo(currentRomPath).fileName());
        }
        showGame();
        return;
    }
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
    if (linkActiveValue) {
        stopLink();
    }
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
    if (!gb) {
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

void AppController::setCurrentRomPath(const QString& path)
{
    QString normalized;
    if (!path.isEmpty()) {
        const QFileInfo info(path);
        normalized = info.canonicalFilePath().isEmpty()
            ? info.absoluteFilePath()
            : info.canonicalFilePath();
    }
    if (currentRomPath == normalized) {
        return;
    }
    currentRomPath = normalized;
    emit currentGamePathChanged();
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
    for (EmulatorItem* item : linkDisplayItems) {
        if (item) {
            item->setKeyBindings(settingsValue->activeKeyBindings());
        }
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
