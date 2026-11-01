/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QFutureWatcher>
#include <QHash>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QSortFilterProxyModel>
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

struct ScanResult
{
    QString path;
    GbCartridgeHeader header;
    RomHash hash;
    QString archiveEntry;
    QString error;
    bool valid = false;
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
 * Case-insensitive, space-separated search over the library metadata.
 *
 * Each term must match at least one of title/publisher/developer/region/
 * language/database name/archive entry/serial/genre/status/checksum/platform.
 */
class GameFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit GameFilterProxyModel(QObject* parent = nullptr);

    void setFilterText(const QString& text);
    int count() const { return rowCount(); }

signals:
    void countChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    QString filterTextValue;
};

/**
 * Per-platform ROM library.
 *
 * Files under the configured platform directories are scanned directly.
 * ROMs opened from outside those directories are kept as references, so the
 * user does not have to copy their collection into an app-managed folder.
 *
 * The scan runs on Qt's thread pool; the UI keeps showing a loading state
 * until the metadata results are merged back on the GUI thread.
 */
class GameLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel* gbGames READ gbGames CONSTANT)
    Q_PROPERTY(QAbstractItemModel* gbcGames READ gbcGames CONSTANT)
    Q_PROPERTY(int gbCount READ gbCount NOTIFY countsChanged)
    Q_PROPERTY(int gbcCount READ gbcCount NOTIFY countsChanged)
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText
        NOTIFY filterTextChanged)
    Q_PROPERTY(QString databaseSource READ databaseSource CONSTANT)
    Q_PROPERTY(QString databaseVersion READ databaseVersion CONSTANT)
    Q_PROPERTY(QString databaseLicenseUrl READ databaseLicenseUrl CONSTANT)
    Q_PROPERTY(int databaseCount READ databaseCount CONSTANT)

public:
    explicit GameLibrary(QObject* parent = nullptr,
        const QString& databasePath = QStringLiteral(":/data/romdb.json"));
    ~GameLibrary() override;

    QAbstractItemModel* gbGames() { return &gbProxy; }
    QAbstractItemModel* gbcGames() { return &gbcProxy; }
    int gbCount() const { return gbProxy.count(); }
    int gbcCount() const { return gbcProxy.count(); }
    bool isLoading() const { return loadingValue; }
    QString filterText() const { return filterTextValue; }
    void setFilterText(const QString& text);
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

    /// Block until the current (and any queued) scan has been merged.  Used by
    /// tests and headless callers that do not run an event loop.
    void waitForScan();

signals:
    void countsChanged();
    void loadingChanged();
    void filterTextChanged();

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
    void startScan();
    void finishScan();
    void setLoading(bool value);
    GameEntry makeEntry(const QString& path, const GbCartridgeHeader& header,
        const RomHash& hash, const QString& archiveEntry) const;
    GameEntry makeFallbackEntry(const QString& path) const;
    void applyDatabase(GameEntry& entry, const RomHash& hash) const;
    static bool isUnder(const QString& path, const QString& directory);

    GameListModel gbModel;
    GameListModel gbcModel;
    GameFilterProxyModel gbProxy;
    GameFilterProxyModel gbcProxy;
    RomDatabase database;
    QFutureWatcher<ScanResult> scanWatcher;
    QMap<QString, GameEntry> scanEntries;
    QHash<QString, HashCacheEntry> hashCache;
    QString gbLibraryPath;
    QString gbcLibraryPath;
    QStringList references;
    QStringList hiddenPaths;
    QString filterTextValue;
    QSettings settings;
    bool loadingValue = false;
    bool scanPending = false;
    bool scanHandled = true;
    bool scanHasFuture = false;
};

}  // namespace qnemu
