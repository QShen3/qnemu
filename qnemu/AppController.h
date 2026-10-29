/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QUrl>

#include "qnemu/gb/Gb.h"

namespace qnemu
{

class EmulatorItem;

/**
 * C++ side of the QML shell.
 *
 * It owns the Gb system and the audio sink, and exposes only small properties
 * and invokable actions to QML.  The QML item implementing DisplayInterface /
 * InputInterface is attached once at startup.
 */
class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool hasGame READ hasGame NOTIFY hasGameChanged)

public:
    explicit AppController(GbModel model, QObject* parent = nullptr);
    ~AppController() override = default;

    QString windowTitle() const { return title; }
    bool hasGame() const { return gb != nullptr; }

    Q_INVOKABLE void attachDisplay(QObject* item);
    Q_INVOKABLE void openRom(const QUrl& url);

signals:
    void windowTitleChanged();
    void hasGameChanged();

private:
    void setWindowTitle(const QString& newTitle);

    GbModel model;
    EmulatorItem* displayItem = nullptr;
    std::unique_ptr<Gb> gb;
    QString title = QStringLiteral("QNEmu");
};

}  // namespace qnemu
