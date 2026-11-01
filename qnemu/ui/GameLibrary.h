/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVector>

#include "qnemu/gb/cartridge/GbCartridgeHeader.h"
#include "qnemu/ui/RomDatabase.h"

namespace qnemu
{

struct GameEntry
{
    QString title;
    QString publisher;
    QString developer;
    QString region;
    QString languages;
    QString serial;
    QString genre;
    QString databaseName;
    QString archiveEntry;
    QString crc32;
    QString sha1;
    QString path;
    QString platform;
    QString status;
    int year = 0;
    bool languagesInferred = false;
    bool official = false;
    bool verified = false;
    bool missing = false;
};

class GameListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role
    {
        TitleRole = Qt::UserRole + 1,
        PublisherRole,
        DeveloperRole,
        RegionRole,
        LanguagesRole,
        LanguagesInferredRole,
        SerialRole,
        PathRole,
        PlatformRole,
        StatusRole,
        OfficialRole,
        VerifiedRole,
        CrcRole,
        Sha1Role,
        DatabaseNameRole,
        ArchiveEntryRole,
        GenreRole,
        YearRole,
        MissingRole,
    };

    explicit GameListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return games.size(); }
    void setGames(QVector<GameEntry> newGames);

signals:
    void countChanged();

private:
    QVector<GameEntry> games;
};

/**
 * Per-platform ROM library.
 *
 * Files under the configured platform directories are scanned directly.
 * ROMs opened from outside those directories are kept as references, so the
 * user does not have to copy their collection into an app-managed folder.
 */
class GameLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractListModel* gbGames READ gbGames CONSTANT)
    Q_PROPERTY(QAbstractListModel* gbcGames READ gbcGames CONSTANT)
    Q_PROPERTY(int gbCount READ gbCount NOTIFY countsChanged)
    Q_PROPERTY(int gbcCount READ gbcCount NOTIFY countsChanged)
    Q_PROPERTY(QString databaseSource READ databaseSource CONSTANT)
    Q_PROPERTY(QString databaseVersion READ databaseVersion CONSTANT)
    Q_PROPERTY(QString databaseLicenseUrl READ databaseLicenseUrl CONSTANT)
    Q_PROPERTY(int databaseCount READ databaseCount CONSTANT)

public:
    explicit GameLibrary(QObject* parent = nullptr,
        const QString& databasePath = QStringLiteral(":/data/romdb.json"));

    QAbstractListModel* gbGames() { return &gbModel; }
    QAbstractListModel* gbcGames() { return &gbcModel; }
    int gbCount() const { return gbModel.count(); }
    int gbcCount() const { return gbcModel.count(); }
    QString databaseSource() const { return database.sourceLabel(); }
    QString databaseVersion() const { return database.datVersion(); }
    QString databaseLicenseUrl() const { return database.sourceLicenseUrl(); }
    int databaseCount() const { return database.count(); }

    void setPaths(const QString& gbPath, const QString& gbcPath);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addGame(const QString& path);
    Q_INVOKABLE void addOpenedGame(const QString& path);
    Q_INVOKABLE void removeGame(const QString& path);
    Q_INVOKABLE bool contains(const QString& path) const;

signals:
    void countsChanged();

private:
    struct HashCacheEntry
    {
        qint64 size = 0;
        qint64 modified = 0;
        RomHash hash;
        GbCartridgeHeader header;
        QString archiveEntry;
        bool valid = false;
    };

    void loadLists();
    void saveLists();
    void scan();
    GameEntry makeEntry(const QString& path);
    void applyDatabase(GameEntry& entry, const RomHash& hash) const;
    static bool isUnder(const QString& path, const QString& directory);

    GameListModel gbModel;
    GameListModel gbcModel;
    RomDatabase database;
    QHash<QString, HashCacheEntry> hashCache;
    QString gbLibraryPath;
    QString gbcLibraryPath;
    QStringList references;
    QStringList hiddenPaths;
    QSettings settings;
};

}  // namespace qnemu
