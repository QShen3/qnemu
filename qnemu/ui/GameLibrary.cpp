/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <utility>

#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFileInfo>
#include <QtCore/QMap>
#include <QtCore/QVariant>

#include "qnemu/ui/GameLibrary.h"
#include "qnemu/gb/cartridge/GbCartridge.h"

namespace qnemu
{

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
    case RegionRole:
        return game.region;
    case PathRole:
        return game.path;
    case PlatformRole:
        return game.platform;
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
        {RegionRole, QByteArrayLiteral("region")},
        {PathRole, QByteArrayLiteral("path")},
        {PlatformRole, QByteArrayLiteral("platform")},
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

GameLibrary::GameLibrary(QObject* parent) : QObject(parent)
{
    loadLists();
}

void GameLibrary::setPaths(const QString& gbPath, const QString& gbcPath)
{
    gbLibraryPath = QDir::cleanPath(gbPath);
    gbcLibraryPath = QDir::cleanPath(gbcPath);
    scan();
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
    QMap<QString, GameEntry> entries;
    const auto addPath = [this, &entries](const QString& path) {
        const QFileInfo info(path);
        const QString absolute = info.canonicalFilePath().isEmpty()
            ? info.absoluteFilePath()
            : info.canonicalFilePath();
        if (absolute.isEmpty() || hiddenPaths.contains(absolute)) {
            return;
        }
        entries.insert(absolute, makeEntry(absolute));
    };

    const auto scanDirectory = [&addPath](const QString& root) {
        if (root.isEmpty() || !QDir(root).exists()) {
            return;
        }
        QDirIterator iterator(root,
            {QStringLiteral("*.gb"), QStringLiteral("*.gbc"), QStringLiteral("*.sgb")},
            QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            addPath(iterator.next());
        }
    };

    scanDirectory(gbLibraryPath);
    scanDirectory(gbcLibraryPath);
    for (const QString& path : std::as_const(references)) {
        addPath(path);
    }

    QVector<GameEntry> gbGames;
    QVector<GameEntry> gbcGames;
    for (const GameEntry& game : std::as_const(entries)) {
        if (game.platform == QStringLiteral("GBC")) {
            gbcGames.append(game);
        } else {
            gbGames.append(game);
        }
    }

    const auto byTitle = [](const GameEntry& left, const GameEntry& right) {
        return left.title.localeAwareCompare(right.title) < 0;
    };
    std::sort(gbGames.begin(), gbGames.end(), byTitle);
    std::sort(gbcGames.begin(), gbcGames.end(), byTitle);

    gbModel.setGames(std::move(gbGames));
    gbcModel.setGames(std::move(gbcGames));
    emit countsChanged();
}

GameEntry GameLibrary::makeEntry(const QString& path) const
{
    const QFileInfo info(path);
    GameEntry entry;
    entry.path = info.absoluteFilePath();
    entry.platform = info.suffix().compare(QStringLiteral("gbc"), Qt::CaseInsensitive) == 0
        ? QStringLiteral("GBC")
        : QStringLiteral("GB");
    entry.title = info.completeBaseName();
    entry.missing = !info.exists();

    if (entry.missing) {
        entry.publisher = QStringLiteral("Unknown");
        entry.region = QStringLiteral("Unknown");
        return entry;
    }

    try {
        const GbCartridgeHeader header = GbCartridge::readHeader(entry.path.toStdString());
        entry.title = QString::fromStdString(header.title);
        entry.publisher = QString::fromStdString(header.publisher);
        entry.region = QString::fromStdString(header.region);
        // The cartridge header is authoritative: only CGB-only carts belong
        // to the GBC library.  Dual-mode (0x80) carts stay in the GB library
        // so they can be launched as DMG, SGB or CGB.
        entry.platform = header.cgbOnly ? QStringLiteral("GBC") : QStringLiteral("GB");
    } catch (const std::exception&) {
        entry.publisher = QStringLiteral("Unknown");
        entry.region = QStringLiteral("Unknown");
    }
    if (entry.title.isEmpty()) {
        entry.title = info.completeBaseName();
    }
    return entry;
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
