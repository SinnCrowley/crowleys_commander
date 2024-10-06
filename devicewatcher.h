#ifndef DEVICEWATCHER_H
#define DEVICEWATCHER_H

#include <QObject>
#include <QStorageInfo>
#include <QTimer>
#include <QSet>

class DeviceWatcher : public QObject{
    Q_OBJECT
public:
    explicit DeviceWatcher(QObject *parent = nullptr, int interval = 1000);

signals:
    void deviceConnected(const QString &device);
    void deviceDisconnected(const QString &device);

private slots:
    void checkDevices();

private:
    QSet<QString> connectedDevices;
    QTimer *timer;
};

#endif // DEVICEWATCHER_H
