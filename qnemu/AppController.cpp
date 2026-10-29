/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include <exception>
#include <memory>
#include <string>
#include <utility>

#include <QtCore/QDebug>
#include <QtCore/QFileInfo>

#include "qnemu/AppController.h"
#include "qnemu/EmulatorItem.h"
#include "qnemu/display/DisplayInterface.h"
#include "qnemu/input/InputInterface.h"
#include "qnemu/sound/DefaultSound.h"

namespace qnemu
{

AppController::AppController(GbModel model, QObject* parent) :
    QObject(parent),
    model(model)
{
}

void AppController::attachDisplay(QObject* item)
{
    displayItem = qobject_cast<EmulatorItem*>(item);
}

void AppController::openRom(const QUrl& url)
{
    if (!displayItem || !url.isLocalFile()) {
        return;
    }

    const QString path = url.toLocalFile();
    gb.reset();
    emit hasGameChanged();

    try {
        // The QML item outlives the Gb instance; the aliasing shared_ptrs only
        // borrow the display/input interfaces and do not take ownership.
        auto display = std::shared_ptr<DisplayInterface>(displayItem,
            [](DisplayInterface*) {});
        auto input = std::shared_ptr<InputInterface>(displayItem,
            [](InputInterface*) {});

        auto system = std::make_unique<Gb>(model, display,
            std::make_unique<DefaultSound>(), input);
        system->loadCartridge(path.toStdString().c_str());

        gb = std::move(system);
        setWindowTitle(QFileInfo(path).fileName() + QStringLiteral(" - QNEmu"));
        emit hasGameChanged();
    } catch (const std::exception& error) {
        qWarning().noquote() << "Failed to load ROM:" << error.what();
        setWindowTitle(QStringLiteral("QNEmu"));
    }
}

void AppController::setWindowTitle(const QString& newTitle)
{
    if (title == newTitle) {
        return;
    }
    title = newTitle;
    emit windowTitleChanged();
}

}  // namespace qnemu
