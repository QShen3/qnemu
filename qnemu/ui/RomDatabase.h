/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

namespace qnemu
{

struct RomRecord
{
    QString title;
    QString name;
    QString platform;
    QString region;
    QStringList languages;
    bool languagesInferred = false;
    QString publisher;
    QString developer;
    QString genre;
    QString serial;
    int year = 0;
    quint32 size = 0;
    quint32 crc32 = 0;
    QByteArray md5;
    QByteArray sha1;
    QString status;

    bool official() const { return status == QStringLiteral("official"); }
};

struct RomHash
{
    quint64 size = 0;
    quint32 crc32 = 0;
    QByteArray sha1;
    bool valid = false;
};

/**
 * Built-in No-Intro metadata index.
 *
 * Entries are keyed by SHA-1.  CRC32 and size are kept for display and for a
 * cheap sanity check; SHA-1 is what decides whether a file is a known dump.
 */
class RomDatabase
{
public:
    bool load(const QString& path, QString* error = nullptr);

    const RomRecord* find(const QByteArray& sha1) const;
    bool hasSize(quint32 size) const { return sizes.contains(size); }
    int count() const { return records.size(); }

    QString source() const { return sourceValue; }
    QString sourceUrl() const { return sourceUrlValue; }
    QString sourceCommit() const { return sourceCommitValue; }
    QString sourceLicense() const { return sourceLicenseValue; }
    QString sourceLicenseUrl() const { return sourceLicenseUrlValue; }
    QString datVersion() const { return datVersionValue; }
    QString generated() const { return generatedValue; }
    QString sourceLabel() const;

    static RomHash hashFile(const QString& path);
    static RomHash hashData(const QByteArray& data);
    static quint32 crc32(const QByteArray& data);

private:
    QVector<RomRecord> records;
    QHash<QByteArray, int> bySha1;
    QSet<quint32> sizes;
    QString sourceValue;
    QString sourceUrlValue;
    QString sourceCommitValue;
    QString sourceLicenseValue;
    QString sourceLicenseUrlValue;
    QString datVersionValue;
    QString generatedValue;
};

}  // namespace qnemu
