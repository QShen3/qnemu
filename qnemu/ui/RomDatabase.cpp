/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <array>
#include <utility>

#include <QtCore/QCryptographicHash>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonValue>

#include "qnemu/ui/RomDatabase.h"

namespace qnemu
{

namespace
{

const std::array<quint32, 256>& crcTable()
{
    static const std::array<quint32, 256> table = [] {
        std::array<quint32, 256> values {};
        for (std::size_t index = 0; index < values.size(); ++index) {
            quint32 value = static_cast<quint32>(index);
            for (int bit = 0; bit < 8; ++bit) {
                value = (value & 1U) != 0 ? (0xEDB88320U ^ (value >> 1U)) : (value >> 1U);
            }
            values[index] = value;
        }
        return values;
    }();
    return table;
}

quint32 updateCrc32(quint32 crc, const QByteArray& data)
{
    const auto& table = crcTable();
    const auto* bytes = reinterpret_cast<const uchar*>(data.constData());
    for (qsizetype index = 0; index < data.size(); ++index) {
        crc = table[(crc ^ bytes[index]) & 0xFFU] ^ (crc >> 8U);
    }
    return crc;
}

QString jsonString(const QJsonObject& object, const QString& key)
{
    return object.value(key).toString();
}

}  // namespace

bool RomDatabase::load(const QString& path, QString* error)
{
    records.clear();
    bySha1.clear();
    sizes.clear();
    sourceValue.clear();
    sourceUrlValue.clear();
    sourceCommitValue.clear();
    sourceLicenseValue.clear();
    sourceLicenseUrlValue.clear();
    datVersionValue.clear();
    generatedValue.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error != nullptr) {
            *error = QStringLiteral("Cannot open game database: %1").arg(path);
        }
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error != nullptr) {
            *error = QStringLiteral("Invalid game database: %1").arg(parseError.errorString());
        }
        return false;
    }

    const QJsonObject root = document.object();
    if (root.value(QStringLiteral("schema")).toInt() != 1) {
        if (error != nullptr) {
            *error = QStringLiteral("Unsupported game database schema");
        }
        return false;
    }

    const QJsonArray entries = root.value(QStringLiteral("entries")).toArray();
    if (entries.isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("Game database is empty");
        }
        return false;
    }

    QVector<RomRecord> loaded;
    loaded.reserve(entries.size());
    QHash<QByteArray, int> index;
    QSet<quint32> knownSizes;
    qsizetype skipped = 0;

    for (const QJsonValue& value : entries) {
        const QJsonObject object = value.toObject();
        RomRecord record;
        record.title = jsonString(object, QStringLiteral("title"));
        record.name = jsonString(object, QStringLiteral("name"));
        record.platform = jsonString(object, QStringLiteral("platform"));
        record.region = jsonString(object, QStringLiteral("region"));
        const QJsonArray languages = object.value(QStringLiteral("languages")).toArray();
        for (const QJsonValue& language : languages) {
            const QString code = language.toString();
            if (!code.isEmpty()) {
                record.languages.append(code);
            }
        }
        record.languagesInferred = object.value(QStringLiteral("languagesInferred")).toBool();
        record.publisher = jsonString(object, QStringLiteral("publisher"));
        record.developer = jsonString(object, QStringLiteral("developer"));
        record.genre = jsonString(object, QStringLiteral("genre"));
        record.serial = jsonString(object, QStringLiteral("serial"));
        record.year = object.value(QStringLiteral("year")).toInt();
        record.size = static_cast<quint32>(object.value(QStringLiteral("size")).toDouble());
        record.md5 = QByteArray::fromHex(jsonString(object, QStringLiteral("md5")).toLatin1());
        record.sha1 = QByteArray::fromHex(jsonString(object, QStringLiteral("sha1")).toLatin1());
        record.status = jsonString(object, QStringLiteral("status"));
        if (record.status.isEmpty()) {
            record.status = object.value(QStringLiteral("official")).toBool()
                ? QStringLiteral("official")
                : QStringLiteral("unknown");
        }

        bool crcOk = false;
        record.crc32 = jsonString(object, QStringLiteral("crc32")).toUInt(&crcOk, 16);
        if (!crcOk || record.size == 0 || record.sha1.size() != 20 || record.title.isEmpty()) {
            ++skipped;
            continue;
        }

        const int position = loaded.size();
        loaded.append(std::move(record));
        index.insert(loaded.constLast().sha1, position);
        knownSizes.insert(loaded.constLast().size);
    }

    if (loaded.isEmpty()) {
        if (error != nullptr) {
            *error = QStringLiteral("Game database contains no valid entries");
        }
        return false;
    }
    if (skipped > 0) {
        qWarning() << "RomDatabase: skipped" << skipped << "invalid entries";
    }

    records = std::move(loaded);
    bySha1 = std::move(index);
    sizes = std::move(knownSizes);

    sourceValue = jsonString(root, QStringLiteral("source"));
    sourceUrlValue = jsonString(root, QStringLiteral("sourceUrl"));
    sourceCommitValue = jsonString(root, QStringLiteral("sourceCommit"));
    sourceLicenseValue = jsonString(root, QStringLiteral("sourceLicense"));
    sourceLicenseUrlValue = jsonString(root, QStringLiteral("sourceLicenseUrl"));
    generatedValue = jsonString(root, QStringLiteral("generated"));

    const QJsonObject versions = root.value(QStringLiteral("datVersions")).toObject();
    QStringList versionParts;
    for (auto it = versions.begin(); it != versions.end(); ++it) {
        versionParts.append(QStringLiteral("%1 %2").arg(it.key(), it.value().toString()));
    }
    datVersionValue = versionParts.join(QStringLiteral(", "));
    if (datVersionValue.isEmpty()) {
        datVersionValue = jsonString(root, QStringLiteral("datVersion"));
    }

    return true;
}

const RomRecord* RomDatabase::find(const QByteArray& sha1) const
{
    const auto it = bySha1.constFind(sha1);
    if (it == bySha1.constEnd()) {
        return nullptr;
    }
    return &records.at(*it);
}

QString RomDatabase::sourceLabel() const
{
    if (sourceValue.isEmpty()) {
        return datVersionValue;
    }
    if (datVersionValue.isEmpty()) {
        return sourceValue;
    }
    return QStringLiteral("%1 %2").arg(sourceValue, datVersionValue);
}

RomHash RomDatabase::hashFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash sha1(QCryptographicHash::Sha1);
    quint32 crc = 0xFFFFFFFFU;
    quint64 size = 0;
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(1 << 20);
        if (chunk.isEmpty() && file.error() != QFileDevice::NoError) {
            return {};
        }
        size += static_cast<quint64>(chunk.size());
        crc = updateCrc32(crc, chunk);
        sha1.addData(chunk);
    }

    RomHash hash;
    hash.size = size;
    hash.crc32 = crc ^ 0xFFFFFFFFU;
    hash.sha1 = sha1.result();
    hash.valid = true;
    return hash;
}

RomHash RomDatabase::hashData(const QByteArray& data)
{
    RomHash hash;
    hash.size = static_cast<quint64>(data.size());
    hash.crc32 = crc32(data);
    hash.sha1 = QCryptographicHash::hash(data, QCryptographicHash::Sha1);
    hash.valid = true;
    return hash;
}

quint32 RomDatabase::crc32(const QByteArray& data)
{
    return updateCrc32(0xFFFFFFFFU, data) ^ 0xFFFFFFFFU;
}

}  // namespace qnemu
