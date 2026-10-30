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

namespace qnemu
{

struct GameEntry
{
    QString title;
    QString publisher;
    QString region;
    QString path;
    QString platform;
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
        RegionRole,
        PathRole,
        PlatformRole,
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

public:
    explicit GameLibrary(QObject* parent = nullptr);

    QAbstractListModel* gbGames() { return &gbModel; }
    QAbstractListModel* gbcGames() { return &gbcModel; }
    int gbCount() const { return gbModel.count(); }
    int gbcCount() const { return gbcModel.count(); }

    void setPaths(const QString& gbPath, const QString& gbcPath);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void addGame(const QString& path);
    Q_INVOKABLE void addOpenedGame(const QString& path);
    Q_INVOKABLE void removeGame(const QString& path);
    Q_INVOKABLE bool contains(const QString& path) const;

signals:
    void countsChanged();

private:
    void loadLists();
    void saveLists();
    void scan();
    GameEntry makeEntry(const QString& path) const;
    static bool isUnder(const QString& path, const QString& directory);

    GameListModel gbModel;
    GameListModel gbcModel;
    QString gbLibraryPath;
    QString gbcLibraryPath;
    QStringList references;
    QStringList hiddenPaths;
    QSettings settings;
};

}  // namespace qnemu
