#ifndef BTCOM_H
#define BTCOM_H

#include <QBluetoothLocalDevice>
#include <QBluetoothAddress>
#include <QBluetoothDeviceInfo>
#include <QList>

class BtCom : public QObject
{
    Q_OBJECT

public:
    BtCom();

private slots:
    void deviceDiscovery( const QBluetoothDeviceInfo& btdi);

private:
    QBluetoothLocalDevice       m_btld;
    QList<QBluetoothDeviceInfo> m_btdi;
    int                         m_ixbtdi;
};

#endif // BTCOM_H
