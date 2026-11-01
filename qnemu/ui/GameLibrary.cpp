/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include <QtConcurrent/QtConcurrent>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QVariant>

#include "qnemu/ui/GameLibrary.h"
#include "qnemu/ui/RomArchive.h"
#include "qnemu/gb/cartridge/GbCartridge.h"

namespace qnemu
{

namespace
{

QString languageName(const QString& code)
{
    static const QHash<QString, QString> names = {
        {QStringLiteral("En"), QStringLiteral("English")},
        {QStringLiteral("Ja"), QStringLiteral("Japanese")},
        {QStringLiteral("Fr"), QStringLiteral("French")},
        {QStringLiteral("De"), QStringLiteral("German")},
        {QStringLiteral("Es"), QStringLiteral("Spanish")},
        {QStringLiteral("It"), QStringLiteral("Italian")},
        {QStringLiteral("Nl"), QStringLiteral("Dutch")},
        {QStringLiteral("Pt"), QStringLiteral("Portuguese")},
        {QStringLiteral("Sv"), QStringLiteral("Swedish")},
        {QStringLiteral("No"), QStringLiteral("Norwegian")},
        {QStringLiteral("Da"), QStringLiteral("Danish")},
        {QStringLiteral("Fi"), QStringLiteral("Finnish")},
        {QStringLiteral("Zh"), QStringLiteral("Chinese")},
        {QStringLiteral("Ko"), QStringLiteral("Korean")},
        {QStringLiteral("Pl"), QStringLiteral("Polish")},
        {QStringLiteral("Ru"), QStringLiteral("Russian")},
    };
    return names.value(code, code);
}

QString languageList(const QStringList& codes)
{
    QStringList names;
    names.reserve(codes.size());
    for (const QString& code : codes) {
        names.append(languageName(code));
    }
    return names.join(QStringLiteral(", "));
}

QString inferredLanguages(const QString& region)
{
    if (region == QStringLiteral("Japan")) {
        return QStringLiteral("Japanese");
    }
    if (region == QStringLiteral("World")) {
        return QStringLiteral("English");
    }
    return {};
}

void applyHeader(GameEntry& entry, const GbCartridgeHeader& header)
{
    entry.title = QString::fromStdString(header.title);
    entry.publisher = QString::fromStdString(header.publisher);
    entry.region = QString::fromStdString(header.region);
    // The cartridge header is authoritative: only CGB-only carts belong to
    // the GBC library.  Dual-mode (0x80) carts stay in the GB library so they
    // can be launched as DMG, SGB or CGB.
    entry.platform = header.cgbOnly ? QStringLiteral("GBC") : QStringLiteral("GB");
}

void applyHash(GameEntry& entry, const RomHash& hash)
{
    entry.crc32 = QString::number(hash.crc32, 16)
        .rightJustified(8, QLatin1Char('0'))
        .toUpper();
    entry.sha1 = QString::fromLatin1(hash.sha1.toHex().toUpper());
}

void finishEntry(GameEntry& entry, const QFileInfo& info)
{
    if (entry.languages.isEmpty()) {
        entry.languages = inferredLanguages(entry.region);
        entry.languagesInferred = !entry.languages.isEmpty();
    }
    if (!entry.verified) {
        entry.status = QStringLiteral("unknown");
    }
    if (entry.title.isEmpty()) {
        entry.title = info.completeBaseName();
    }
}

bool isSupportedRomPath(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QStringLiteral("gb") || suffix == QStringLiteral("gbc")
        || suffix == QStringLiteral("sgb") || RomArchive::isArchivePath(path);
}

ScanResult scanPath(const QString& path, const RomDatabase& database)
{
    ScanResult result;
    result.path = path;
    try {
        const QFileInfo info(path);
        if (!info.exists()) {
            result.error = QStringLiteral("File missing");
            return result;
        }

        if (RomArchive::isArchivePath(path)) {
            RomArchiveEntry archive;
            QString error;
            if (!RomArchive::read(path, &archive, &error)) {
                result.error = error;
                return result;
            }
            result.archiveEntry = archive.name;
            const std::vector<uint8_t> romData(archive.data.begin(), archive.data.end());
            result.header = GbCartridge::readHeader(romData);
            result.hash = RomDatabase::hashData(archive.data);
        } else {
            result.header = GbCartridge::readHeader(path.toStdString());
            const quint32 fileSize = static_cast<quint32>(info.size());
            if (database.hasSize(fileSize)) {
                result.hash = RomDatabase::hashFile(path);
            }
        }
        result.valid = true;
    } catch (const std::exception& error) {
        result.error = QString::fromUtf8(error.what());
    }
    return result;
}

}  // namespace

GameListModel::GameListModel(QObject* parent) : QAbstractListModel(parent)
{
}

int GameListModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : games.size();
}

QVariant GameListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= games.size()) {
        return {};
    }

    const GameEntry& game = games.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return game.title;
    case PublisherRole:
        return game.publisher;
    case DeveloperRole:
        return game.developer;
    case RegionRole:
        return game.region;
    case LanguagesRole:
        return game.languages;
    case LanguagesInferredRole:
        return game.languagesInferred;
    case SerialRole:
        return game.serial;
    case PathRole:
        return game.path;
    case PlatformRole:
        return game.platform;
    case StatusRole:
        return game.status;
    case OfficialRole:
        return game.official;
    case VerifiedRole:
        return game.verified;
    case CrcRole:
        return game.crc32;
    case Sha1Role:
        return game.sha1;
    case DatabaseNameRole:
        return game.databaseName;
    case ArchiveEntryRole:
        return game.archiveEntry;
    case GenreRole:
        return game.genre;
    case YearRole:
        return game.year;
    case MissingRole:
        return game.missing;
    default:
        return {};
    }
}

QHash<int, QByteArray> GameListModel::roleNames() const
{
    return {
        {TitleRole, QByteArrayLiteral("title")},
        {PublisherRole, QByteArrayLiteral("publisher")},
        {DeveloperRole, QByteArrayLiteral("developer")},
        {RegionRole, QByteArrayLiteral("region")},
        {LanguagesRole, QByteArrayLiteral("languages")},
        {LanguagesInferredRole, QByteArrayLiteral("languagesInferred")},
        {SerialRole, QByteArrayLiteral("serial")},
        {PathRole, QByteArrayLiteral("path")},
        {PlatformRole, QByteArrayLiteral("platform")},
        {StatusRole, QByteArrayLiteral("status")},
        {OfficialRole, QByteArrayLiteral("official")},
        {VerifiedRole, QByteArrayLiteral("verified")},
        {CrcRole, QByteArrayLiteral("crc32")},
        {Sha1Role, QByteArrayLiteral("sha1")},
        {DatabaseNameRole, QByteArrayLiteral("databaseName")},
        {ArchiveEntryRole, QByteArrayLiteral("archiveEntry")},
        {GenreRole, QByteArrayLiteral("genre")},
        {YearRole, QByteArrayLiteral("year")},
        {MissingRole, QByteArrayLiteral("missing")},
    };
}

void GameListModel::setGames(QVector<GameEntry> newGames)
{
    beginResetModel();
    games = std::move(newGames);
    endResetModel();
    emit countChanged();
}

GameFilterProxyModel::GameFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &QAbstractItemModel::rowsInserted,
        this, &GameFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved,
        this, &GameFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset,
        this, &GameFilterProxyModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged,
        this, &GameFilterProxyModel::countChanged);
}

void GameFilterProxyModel::setFilterText(const QString& text)
{
    if (filterTextValue == text) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    beginFilterChange();
    filterTextValue = text;
    endFilterChange();
#else
    filterTextValue = text;
    invalidateFilter();
#endif
}

bool GameFilterProxyModel::filterAcceptsRow(int sourceRow,
    const QModelIndex& sourceParent) const
{
    const QStringList terms = filterTextValue.trimmed().split(
        QLatin1Char(' '), Qt::SkipEmptyParts);
    if (terms.isEmpty()) {
        return true;
    }

    const QAbstractItemModel* model = sourceModel();
    if (model == nullptr) {
        return true;
    }
    const QModelIndex index = model->index(sourceRow, 0, sourceParent);
    QStringList fields;
    for (const int role : {
             GameListModel::TitleRole,
             GameListModel::PublisherRole,
             GameListModel::DeveloperRole,
             GameListModel::RegionRole,
             GameListModel::LanguagesRole,
             GameListModel::SerialRole,
             GameListModel::PlatformRole,
             GameListModel::StatusRole,
             GameListModel::CrcRole,
             GameListModel::Sha1Role,
             GameListModel::DatabaseNameRole,
             GameListModel::ArchiveEntryRole,
             GameListModel::GenreRole,
             GameListModel::YearRole,
         }) {
        const QString value = model->data(index, role).toString();
        if (!value.isEmpty()) {
            fields.append(value);
        }
    }
    const QString haystack = fields.join(QLatin1Char(' '));
    for (const QString& term : terms) {
        if (!haystack.contains(term, Qt::CaseInsensitive)) {
            return false;
        }
    }
    return true;
}

GameLibrary::GameLibrary(QObject* parent, const QString& databasePath) : QObject(parent)
{
    gbProxy.setSourceModel(&gbModel);
    gbcProxy.setSourceModel(&gbcModel);
    connect(&gbProxy, &GameFilterProxyModel::countChanged,
        this, &GameLibrary::countsChanged);
    connect(&gbcProxy, &GameFilterProxyModel::countChanged,
        this, &GameLibrary::countsChanged);
    connect(&scanWatcher, &QFutureWatcher<ScanResult>::finished,
        this, &GameLibrary::finishScan);

    if (!databasePath.isEmpty()) {
        QString error;
        if (!database.load(databasePath, &error)) {
            qWarning() << "Game database not loaded:" << error;
        }
    }
    loadLists();
}

GameLibrary::~GameLibrary()
{
    scanPending = false;
    if (scanHasFuture && scanWatcher.isRunning()) {
        scanWatcher.waitForFinished();
    }
}

void GameLibrary::setPaths(const QString& gbPath, const QString& gbcPath)
{
    gbLibraryPath = QDir::cleanPath(gbPath);
    gbcLibraryPath = QDir::cleanPath(gbcPath);
    scan();
}

void GameLibrary::setFilterText(const QString& text)
{
    if (filterTextValue == text) {
        return;
    }
    filterTextValue = text;
    gbProxy.setFilterText(text);
    gbcProxy.setFilterText(text);
    emit filterTextChanged();
}

void GameLibrary::refresh()
{
    scan();
}

void GameLibrary::addGame(const QString& path)
{
    const QFileInfo info(path);
    const QString absolute = info.canonicalFilePath().isEmpty()
        ? info.absoluteFilePath()
        : info.canonicalFilePath();
    if (absolute.isEmpty()) {
        return;
    }

    hiddenPaths.removeAll(absolute);
    if (!isUnder(absolute, gbLibraryPath) && !isUnder(absolute, gbcLibraryPath)
        && !references.contains(absolute)) {
        references.append(absolute);
    }
    saveLists();
    scan();
}

void GameLibrary::addOpenedGame(const QString& path)
{
    if (!contains(path)) {
        addGame(path);
        return;
    }
    refresh();
}

void GameLibrary::removeGame(const QString& path)
{
    const QFileInfo info(path);
    const QString absolute = info.canonicalFilePath().isEmpty()
        ? info.absoluteFilePath()
        : info.canonicalFilePath();
    if (absolute.isEmpty()) {
        return;
    }

    references.removeAll(absolute);
    if (isUnder(absolute, gbLibraryPath) || isUnder(absolute, gbcLibraryPath)) {
        if (!hiddenPaths.contains(absolute)) {
            hiddenPaths.append(absolute);
        }
    }
    saveLists();
    scan();
}

bool GameLibrary::contains(const QString& path) const
{
    const QFileInfo info(path);
    const QString absolute = info.canonicalFilePath().isEmpty()
        ? info.absoluteFilePath()
        : info.canonicalFilePath();
    if (absolute.isEmpty()) {
        return false;
    }

    for (const GameListModel* model : {&gbModel, &gbcModel}) {
        for (int row = 0; row < model->count(); ++row) {
            if (model->data(model->index(row), GameListModel::PathRole).toString() == absolute) {
                return true;
            }
        }
    }
    return false;
}

void GameLibrary::loadLists()
{
    references = settings.value(QStringLiteral("library/references")).toStringList();
    hiddenPaths = settings.value(QStringLiteral("library/hidden")).toStringList();
}

void GameLibrary::saveLists()
{
    settings.setValue(QStringLiteral("library/references"), references);
    settings.setValue(QStringLiteral("library/hidden"), hiddenPaths);
    settings.sync();
}

void GameLibrary::scan()
{
    if (!scanHandled || (scanHasFuture && scanWatcher.isRunning())) {
        scanPending = true;
        return;
    }
    startScan();
}

void GameLibrary::startScan()
{
    scanHandled = false;
    scanHasFuture = false;
    scanEntries.clear();
    setLoading(true);

    QSet<QString> seen;
    QStringList pending;
    const auto addPath = [this, &seen, &pending](const QString& path) {
        const QFileInfo info(path);
        const QString absolute = info.canonicalFilePath().isEmpty()
            ? info.absoluteFilePath()
            : info.canonicalFilePath();
        if (absolute.isEmpty() || hiddenPaths.contains(absolute)
            || seen.contains(absolute)) {
            return;
        }
        seen.insert(absolute);

        const qint64 modified = info.lastModified().toMSecsSinceEpoch();
        const auto cached = hashCache.constFind(absolute);
        if (cached != hashCache.constEnd() && cached->size == info.size()
            && cached->modified == modified && cached->valid) {
            scanEntries.insert(absolute,
                makeEntry(absolute, cached->header, cached->hash,
                    cached->archiveEntry));
            return;
        }
        pending.append(absolute);
    };

    const auto scanDirectory = [&addPath](const QString& root) {
        if (root.isEmpty() || !QDir(root).exists()) {
            return;
        }
        QDirIterator iterator(root, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const QString path = iterator.next();
            if (isSupportedRomPath(path)) {
                addPath(path);
            }
        }
    };

    scanDirectory(gbLibraryPath);
    scanDirectory(gbcLibraryPath);
    for (const QString& path : std::as_const(references)) {
        addPath(path);
    }

    if (pending.isEmpty()) {
        finishScan();
        return;
    }

    const RomDatabase* databasePtr = &database;
    scanHasFuture = true;
    scanWatcher.setFuture(QtConcurrent::mapped(pending,
        [databasePtr](const QString& path) {
            return scanPath(path, *databasePtr);
        }));
}

void GameLibrary::finishScan()
{
    if (scanHandled) {
        return;
    }
    scanHandled = true;

    if (scanHasFuture && scanWatcher.isFinished()
        && scanWatcher.future().isValid()) {
        const QFuture<ScanResult> future = scanWatcher.future();
        for (int index = 0; index < future.resultCount(); ++index) {
            const ScanResult result = future.resultAt(index);
            if (!result.valid) {
                qWarning().noquote() << "Failed to read ROM metadata:"
                                     << result.path << result.error;
                scanEntries.insert(result.path, makeFallbackEntry(result.path));
                continue;
            }

            scanEntries.insert(result.path,
                makeEntry(result.path, result.header, result.hash,
                    result.archiveEntry));

            const QFileInfo info(result.path);
            HashCacheEntry cache;
            cache.size = info.size();
            cache.modified = info.lastModified().toMSecsSinceEpoch();
            cache.hash = result.hash;
            cache.header = result.header;
            cache.archiveEntry = result.archiveEntry;
            cache.valid = true;
            hashCache.insert(result.path, cache);
        }
    }
    scanHasFuture = false;

    QVector<GameEntry> gbGames;
    QVector<GameEntry> gbcGames;
    for (const GameEntry& game : std::as_const(scanEntries)) {
        if (game.platform == QStringLiteral("GBC")) {
            gbcGames.append(game);
        } else {
            gbGames.append(game);
        }
    }

    const auto byTitle = [](const GameEntry& left, const GameEntry& right) {
        const int titleOrder = left.title.localeAwareCompare(right.title);
        if (titleOrder != 0) {
            return titleOrder < 0;
        }
        const int regionOrder = left.region.localeAwareCompare(right.region);
        if (regionOrder != 0) {
            return regionOrder < 0;
        }
        return left.databaseName.localeAwareCompare(right.databaseName) < 0;
    };
    std::sort(gbGames.begin(), gbGames.end(), byTitle);
    std::sort(gbcGames.begin(), gbcGames.end(), byTitle);

    gbModel.setGames(std::move(gbGames));
    gbcModel.setGames(std::move(gbcGames));
    emit countsChanged();

    if (scanPending) {
        scanPending = false;
        startScan();
        return;
    }
    setLoading(false);
}

void GameLibrary::setLoading(bool value)
{
    if (loadingValue == value) {
        return;
    }
    loadingValue = value;
    emit loadingChanged();
}

void GameLibrary::waitForScan()
{
    while (!scanHandled || (scanHasFuture && scanWatcher.isRunning())) {
        if (scanHasFuture && scanWatcher.isRunning()) {
            scanWatcher.waitForFinished();
        }
        finishScan();
    }
}

GameEntry GameLibrary::makeEntry(const QString& path,
    const GbCartridgeHeader& header, const RomHash& hash,
    const QString& archiveEntry) const
{
    const QFileInfo info(path);
    GameEntry entry;
    entry.path = info.absoluteFilePath();
    entry.platform = info.suffix().compare(QStringLiteral("gbc"),
                         Qt::CaseInsensitive) == 0
        ? QStringLiteral("GBC")
        : QStringLiteral("GB");
    entry.title = info.completeBaseName();
    entry.missing = !info.exists();

    if (entry.missing) {
        entry.publisher = QStringLiteral("Unknown");
        entry.region = QStringLiteral("Unknown");
        return entry;
    }

    applyHeader(entry, header);
    entry.archiveEntry = archiveEntry;
    if (hash.valid) {
        applyHash(entry, hash);
        applyDatabase(entry, hash);
    }
    finishEntry(entry, info);
    return entry;
}

GameEntry GameLibrary::makeFallbackEntry(const QString& path) const
{
    const QFileInfo info(path);
    GameEntry entry;
    entry.path = info.absoluteFilePath();
    entry.platform = info.suffix().compare(QStringLiteral("gbc"),
                         Qt::CaseInsensitive) == 0
        ? QStringLiteral("GBC")
        : QStringLiteral("GB");
    entry.title = info.completeBaseName();
    entry.publisher = QStringLiteral("Unknown");
    entry.region = QStringLiteral("Unknown");
    entry.status = QStringLiteral("unknown");
    entry.missing = !info.exists();
    return entry;
}

void GameLibrary::applyDatabase(GameEntry& entry, const RomHash& hash) const
{
    const RomRecord* record = database.find(hash.sha1);
    if (record == nullptr || static_cast<quint64>(record->size) != hash.size) {
        return;
    }

    entry.verified = true;
    entry.official = record->official();
    entry.status = record->status;
    if (!record->title.isEmpty()) {
        entry.title = record->title;
    }
    if (!record->publisher.isEmpty()) {
        entry.publisher = record->publisher;
    }
    if (!record->region.isEmpty()) {
        entry.region = record->region;
    }
    entry.developer = record->developer;
    entry.languages = languageList(record->languages);
    entry.languagesInferred = record->languagesInferred;
    entry.serial = record->serial;
    entry.genre = record->genre;
    entry.year = record->year;
    entry.databaseName = record->name;
}

bool GameLibrary::isUnder(const QString& path, const QString& directory)
{
    if (directory.isEmpty()) {
        return false;
    }
    const QString root = QDir::cleanPath(QDir(directory).absolutePath());
    const QString absolute = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    return absolute == root || absolute.startsWith(root + QLatin1Char('/'));
}

}  // namespace qnemu
