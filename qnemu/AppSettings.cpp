/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <utility>

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/Qt>
#include <QtGui/QImageWriter>
#include <QtGui/QKeySequence>

#include "qnemu/AppSettings.h"

namespace qnemu
{

namespace
{

QString makeDirectory(const QString& path)
{
    if (!path.isEmpty()) {
        QDir().mkpath(path);
    }
    return path;
}

}  // namespace

AppSettings::AppSettings(QObject* parent) : QObject(parent)
{
    const QList<QByteArray> formats = QImageWriter::supportedImageFormats();
    for (const QByteArray& format : formats) {
        const QString name = QString::fromLatin1(format).toLower();
        if (!name.isEmpty() && !screenshotFormatList.contains(name)) {
            screenshotFormatList.append(name);
        }
    }
    screenshotFormatList.sort();

    settings.beginGroup(QStringLiteral("paths"));
    gbLibraryPathValue = settings.value(QStringLiteral("gbLibrary")).toString();
    gbcLibraryPathValue = settings.value(QStringLiteral("gbcLibrary")).toString();
    savePathValue = settings.value(QStringLiteral("save")).toString();
    settings.endGroup();

    if (gbLibraryPathValue.isEmpty()) {
        gbLibraryPathValue = defaultLibraryPath(QStringLiteral("GB"));
    }
    if (gbcLibraryPathValue.isEmpty()) {
        gbcLibraryPathValue = defaultLibraryPath(QStringLiteral("GBC"));
    }
    if (savePathValue.isEmpty()) {
        savePathValue = defaultSavePath();
    }
    makeDirectory(gbLibraryPathValue);
    makeDirectory(gbcLibraryPathValue);
    makeDirectory(savePathValue);

    settings.beginGroup(QStringLiteral("screenshot"));
    screenshotDirectoryValue = settings.value(QStringLiteral("directory")).toString();
    screenshotFormatValue = settings.value(QStringLiteral("format")).toString().toLower();
    settings.endGroup();
    if (screenshotDirectoryValue.isEmpty()) {
        screenshotDirectoryValue = defaultScreenshotPath();
    }
    makeDirectory(screenshotDirectoryValue);
    if (!screenshotFormatList.contains(screenshotFormatValue)) {
        screenshotFormatValue = screenshotFormatList.contains(QStringLiteral("png"))
            ? QStringLiteral("png")
            : screenshotFormatList.value(0);
    }

    scalePercentValue = std::clamp(settings.value(QStringLiteral("ui/scalePercent"), 200).toInt(),
        100, 200);
    soundEnabledValue = settings.value(QStringLiteral("audio/enabled"), true).toBool();

    loadKeyProfiles();
}

QString AppSettings::defaultLibraryPath(const QString& platform)
{
    QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (documents.isEmpty()) {
        documents = QDir::homePath();
    }
    return QDir(QDir(documents).filePath(QStringLiteral("QNEmu/Library")))
        .filePath(platform);
}

QString AppSettings::defaultSavePath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) {
        base = QDir::homePath() + QStringLiteral("/.qnemu");
    }
    return QDir(base).filePath(QStringLiteral("saves"));
}

QString AppSettings::defaultScreenshotPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (base.isEmpty()) {
        base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    if (base.isEmpty()) {
        base = QDir::homePath();
    }
    return QDir(base).filePath(QStringLiteral("QNEmu"));
}

QVariantMap AppSettings::defaultBindings()
{
    return {
        {QStringLiteral("up"), static_cast<int>(Qt::Key_W)},
        {QStringLiteral("down"), static_cast<int>(Qt::Key_S)},
        {QStringLiteral("left"), static_cast<int>(Qt::Key_A)},
        {QStringLiteral("right"), static_cast<int>(Qt::Key_D)},
        {QStringLiteral("a"), static_cast<int>(Qt::Key_J)},
        {QStringLiteral("b"), static_cast<int>(Qt::Key_K)},
        {QStringLiteral("select"), static_cast<int>(Qt::Key_2)},
        {QStringLiteral("start"), static_cast<int>(Qt::Key_3)},
    };
}

void AppSettings::setGbLibraryPath(const QString& value)
{
    const QString path = makeDirectory(QDir::cleanPath(value));
    if (path.isEmpty() || path == gbLibraryPathValue) {
        return;
    }
    gbLibraryPathValue = path;
    settings.setValue(QStringLiteral("paths/gbLibrary"), path);
    emit gbLibraryPathChanged();
}

void AppSettings::setGbcLibraryPath(const QString& value)
{
    const QString path = makeDirectory(QDir::cleanPath(value));
    if (path.isEmpty() || path == gbcLibraryPathValue) {
        return;
    }
    gbcLibraryPathValue = path;
    settings.setValue(QStringLiteral("paths/gbcLibrary"), path);
    emit gbcLibraryPathChanged();
}

void AppSettings::setSavePath(const QString& value)
{
    const QString path = makeDirectory(QDir::cleanPath(value));
    if (path.isEmpty() || path == savePathValue) {
        return;
    }
    savePathValue = path;
    settings.setValue(QStringLiteral("paths/save"), path);
    emit savePathChanged();
}

void AppSettings::setScreenshotDirectory(const QString& value)
{
    const QString path = makeDirectory(QDir::cleanPath(value));
    if (path.isEmpty() || path == screenshotDirectoryValue) {
        return;
    }
    screenshotDirectoryValue = path;
    settings.setValue(QStringLiteral("screenshot/directory"), path);
    emit screenshotDirectoryChanged();
}

void AppSettings::setScreenshotFormat(const QString& value)
{
    const QString format = value.toLower();
    if (!screenshotFormatList.contains(format) || format == screenshotFormatValue) {
        return;
    }
    screenshotFormatValue = format;
    settings.setValue(QStringLiteral("screenshot/format"), format);
    emit screenshotFormatChanged();
}

void AppSettings::setScalePercent(int value)
{
    const int clamped = std::clamp(value, 100, 200);
    if (clamped == scalePercentValue) {
        return;
    }
    scalePercentValue = clamped;
    settings.setValue(QStringLiteral("ui/scalePercent"), clamped);
    emit scalePercentChanged();
}

void AppSettings::setSoundEnabled(bool value)
{
    if (value == soundEnabledValue) {
        return;
    }
    soundEnabledValue = value;
    settings.setValue(QStringLiteral("audio/enabled"), value);
    emit soundEnabledChanged();
}

QString AppSettings::keyName(int key) const
{
    if (key == 0) {
        return QStringLiteral("—");
    }
    return QKeySequence(key).toString(QKeySequence::NativeText);
}

QStringList AppSettings::profileNames() const
{
    QStringList names;
    for (const QVariant& profile : keyProfilesValue) {
        const QString name = profile.toMap().value(QStringLiteral("name")).toString();
        if (!name.isEmpty()) {
            names.append(name);
        }
    }
    return names;
}

QVariantMap AppSettings::bindingsForProfile(const QString& name) const
{
    const int index = profileIndex(name);
    if (index < 0) {
        return defaultBindings();
    }
    return keyProfilesValue.at(index).toMap().value(QStringLiteral("bindings")).toMap();
}

int AppSettings::profileIndex(const QString& name) const
{
    for (int i = 0; i < keyProfilesValue.size(); ++i) {
        if (keyProfilesValue.at(i).toMap().value(QStringLiteral("name")).toString() == name) {
            return i;
        }
    }
    return -1;
}

void AppSettings::setActiveKeyProfile(const QString& value)
{
    if (value.isEmpty() || profileIndex(value) < 0 || value == activeKeyProfileValue) {
        return;
    }
    activeKeyProfileValue = value;
    saveKeyProfiles();
    emit activeKeyProfileChanged();
    emit activeKeyBindingsChanged();
}

void AppSettings::setKeyBinding(const QString& profile, const QString& button, int key)
{
    const int index = profileIndex(profile);
    if (index < 0 || button.isEmpty()) {
        return;
    }

    QVariantMap entry = keyProfilesValue.at(index).toMap();
    QVariantMap bindings = entry.value(QStringLiteral("bindings")).toMap();
    bindings.insert(button, key);
    entry.insert(QStringLiteral("bindings"), bindings);
    keyProfilesValue[index] = entry;
    saveKeyProfiles();
    emit keyProfilesChanged();
    if (profile == activeKeyProfileValue) {
        emit activeKeyBindingsChanged();
    }
}

void AppSettings::addKeyProfile(const QString& name)
{
    const QString profileName = name.trimmed();
    if (profileName.isEmpty() || profileIndex(profileName) >= 0) {
        return;
    }

    keyProfilesValue.append(QVariantMap {
        {QStringLiteral("name"), profileName},
        {QStringLiteral("bindings"), bindingsForProfile(activeKeyProfileValue)},
    });
    activeKeyProfileValue = profileName;
    saveKeyProfiles();
    emit keyProfilesChanged();
    emit activeKeyProfileChanged();
    emit activeKeyBindingsChanged();
}

void AppSettings::removeKeyProfile(const QString& name)
{
    if (keyProfilesValue.size() <= 1) {
        return;
    }
    const int index = profileIndex(name);
    if (index < 0) {
        return;
    }

    keyProfilesValue.removeAt(index);
    if (activeKeyProfileValue == name) {
        activeKeyProfileValue = profileNames().value(0);
        emit activeKeyProfileChanged();
        emit activeKeyBindingsChanged();
    }
    saveKeyProfiles();
    emit keyProfilesChanged();
}

void AppSettings::loadKeyProfiles()
{
    settings.beginGroup(QStringLiteral("keys"));
    keyProfilesValue = settings.value(QStringLiteral("profiles")).toList();
    activeKeyProfileValue = settings.value(QStringLiteral("activeProfile")).toString();
    settings.endGroup();

    ensureKeyProfile();
}

void AppSettings::saveKeyProfiles()
{
    settings.setValue(QStringLiteral("keys/profiles"), keyProfilesValue);
    settings.setValue(QStringLiteral("keys/activeProfile"), activeKeyProfileValue);
}

void AppSettings::ensureKeyProfile()
{
    if (keyProfilesValue.isEmpty()) {
        keyProfilesValue.append(QVariantMap {
            {QStringLiteral("name"), QStringLiteral("Default")},
            {QStringLiteral("bindings"), defaultBindings()},
        });
    }

    if (profileIndex(activeKeyProfileValue) < 0) {
        activeKeyProfileValue = profileNames().value(0);
    }
}

}  // namespace qnemu
