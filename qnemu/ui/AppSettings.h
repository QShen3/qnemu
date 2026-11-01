/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

namespace qnemu
{

/**
 * Persistent application settings.
 *
 * Global settings cover the UI scale, audio, screenshots and key profiles.
 * Library and save paths are stored per platform so a future console frontend
 * can add more tabs without changing the storage format.
 */
class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString gbLibraryPath READ gbLibraryPath WRITE setGbLibraryPath NOTIFY gbLibraryPathChanged)
    Q_PROPERTY(QString gbcLibraryPath READ gbcLibraryPath WRITE setGbcLibraryPath NOTIFY gbcLibraryPathChanged)
    Q_PROPERTY(QString savePath READ savePath WRITE setSavePath NOTIFY savePathChanged)
    Q_PROPERTY(QString screenshotDirectory READ screenshotDirectory WRITE setScreenshotDirectory NOTIFY screenshotDirectoryChanged)
    Q_PROPERTY(QString screenshotFormat READ screenshotFormat WRITE setScreenshotFormat NOTIFY screenshotFormatChanged)
    Q_PROPERTY(QStringList screenshotFormats READ screenshotFormats CONSTANT)
    Q_PROPERTY(int scalePercent READ scalePercent WRITE setScalePercent NOTIFY scalePercentChanged)
    Q_PROPERTY(bool soundEnabled READ soundEnabled WRITE setSoundEnabled NOTIFY soundEnabledChanged)
    Q_PROPERTY(QString gbLaunchModel READ gbLaunchModel WRITE setGbLaunchModel NOTIFY gbLaunchModelChanged)

    Q_PROPERTY(QString activeKeyProfile READ activeKeyProfile WRITE setActiveKeyProfile NOTIFY activeKeyProfileChanged)
    Q_PROPERTY(QStringList profileNames READ profileNames NOTIFY keyProfilesChanged)
    Q_PROPERTY(QVariantList keyProfiles READ keyProfiles NOTIFY keyProfilesChanged)
    Q_PROPERTY(QVariantMap activeKeyBindings READ activeKeyBindings NOTIFY activeKeyBindingsChanged)

public:
    explicit AppSettings(QObject* parent = nullptr);

    QString gbLibraryPath() const { return gbLibraryPathValue; }
    QString gbcLibraryPath() const { return gbcLibraryPathValue; }
    QString savePath() const { return savePathValue; }
    QString screenshotDirectory() const { return screenshotDirectoryValue; }
    QString screenshotFormat() const { return screenshotFormatValue; }
    QStringList screenshotFormats() const { return screenshotFormatList; }
    int scalePercent() const { return scalePercentValue; }
    bool soundEnabled() const { return soundEnabledValue; }
    QString gbLaunchModel() const { return gbLaunchModelValue; }

    QString activeKeyProfile() const { return activeKeyProfileValue; }
    QStringList profileNames() const;
    QVariantList keyProfiles() const { return keyProfilesValue; }
    QVariantMap activeKeyBindings() const { return bindingsForProfile(activeKeyProfileValue); }

    void setGbLibraryPath(const QString& value);
    void setGbcLibraryPath(const QString& value);
    void setSavePath(const QString& value);
    void setScreenshotDirectory(const QString& value);
    void setScreenshotFormat(const QString& value);
    void setScalePercent(int value);
    void setSoundEnabled(bool value);
    void setGbLaunchModel(const QString& value);
    void setActiveKeyProfile(const QString& value);

    Q_INVOKABLE QString keyName(int key) const;
    Q_INVOKABLE void setKeyBinding(const QString& profile, const QString& button, int key);
    Q_INVOKABLE void addKeyProfile(const QString& name);
    Q_INVOKABLE void removeKeyProfile(const QString& name);

signals:
    void gbLibraryPathChanged();
    void gbcLibraryPathChanged();
    void savePathChanged();
    void screenshotDirectoryChanged();
    void screenshotFormatChanged();
    void scalePercentChanged();
    void soundEnabledChanged();
    void gbLaunchModelChanged();
    void activeKeyProfileChanged();
    void keyProfilesChanged();
    void activeKeyBindingsChanged();

private:
    static QString defaultLibraryPath(const QString& platform);
    static QString defaultSavePath();
    static QString defaultScreenshotPath();
    static QVariantMap defaultBindings();

    QVariantMap bindingsForProfile(const QString& name) const;
    int profileIndex(const QString& name) const;
    void loadKeyProfiles();
    void saveKeyProfiles();
    void ensureKeyProfile();

    QSettings settings;
    QString gbLibraryPathValue;
    QString gbcLibraryPathValue;
    QString savePathValue;
    QString screenshotDirectoryValue;
    QString screenshotFormatValue;
    QStringList screenshotFormatList;
    int scalePercentValue = 200;
    bool soundEnabledValue = true;
    QString gbLaunchModelValue = QStringLiteral("cgb");

    QVariantList keyProfilesValue;
    QString activeKeyProfileValue;
};

}  // namespace qnemu
