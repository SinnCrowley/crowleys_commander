#include <QStorageInfo>

#include "devicewatcher.h"

DeviceWatcher::DeviceWatcher(QObject *parent, int interval)
    : QObject(parent),
    timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, &DeviceWatcher::checkDevices);
    timer->start(interval);

    // initiation of device list
    checkDevices();
}

void DeviceWatcher::checkDevices()
{
    QSet<QString> currentDevices;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
        if (storage.isValid() && storage.isReady())
            currentDevices.insert(storage.rootPath());
    }

    // check for device connection
    foreach (const QString &device, currentDevices) {
        if (!connectedDevices.contains(device))
            emit deviceConnected(device);
    }

    // check for disconnection
    foreach (const QString &device, connectedDevices) {
        if (!currentDevices.contains(device))
            emit deviceDisconnected(device);
    }

    connectedDevices = currentDevices;
}
