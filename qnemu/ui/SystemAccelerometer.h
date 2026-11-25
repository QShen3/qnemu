/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#pragma once

#include <QObject>

#include <functional>
#include <memory>

namespace qnemu
{

/** Host-side access to the system accelerometer, when Qt Sensors provides one. */
class SystemAccelerometer : public QObject
{
    Q_OBJECT

public:
    static bool available();

    explicit SystemAccelerometer(QObject* parent = nullptr);
    ~SystemAccelerometer() override;

    void setEnabled(bool enabled);
    void setCallback(std::function<void(float, float)> callback);

private:
    class Private;
    std::unique_ptr<Private> d;
    std::function<void(float, float)> callback;
    bool enabled = false;
};

}  // namespace qnemu
