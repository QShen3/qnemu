/*
 *  Copyright [2026] <qazxdrcssc2006@163.com>
 */

#include "qnemu/ui/SystemAccelerometer.h"

#include <utility>

#include <QtSensors/QAccelerometer>
#include <QtSensors/QSensor>

namespace qnemu
{

class SystemAccelerometer::Private
{
public:
    QAccelerometer sensor;
};

SystemAccelerometer::SystemAccelerometer(QObject* parent) : QObject(parent)
{
    d = std::make_unique<Private>();
    d->sensor.setDataRate(50);
    connect(&d->sensor, &QSensor::readingChanged, this, [this]() {
        if (!enabled || !d->sensor.reading() || !callback) {
            return;
        }
        constexpr qreal gravity = 9.80665;
        const qreal x = d->sensor.reading()->property("x").toReal() / gravity;
        const qreal y = d->sensor.reading()->property("y").toReal() / gravity;
        callback(static_cast<float>(-x), static_cast<float>(y));
    });
}

SystemAccelerometer::~SystemAccelerometer() = default;

bool SystemAccelerometer::available()
{
    QAccelerometer sensor;
    return !QSensor::sensorsForType(sensor.type()).isEmpty();
}

void SystemAccelerometer::setEnabled(bool value)
{
    if (enabled == value) {
        return;
    }
    enabled = value && available();
    if (enabled) {
        d->sensor.start();
    } else {
        d->sensor.stop();
    }
}

void SystemAccelerometer::setCallback(std::function<void(float, float)> value)
{
    callback = std::move(value);
}

}  // namespace qnemu
