#include "btcom.h"
#include <QBluetoothDeviceDiscoveryAgent>
#include <QDebug>

BtCom::BtCom()
{
    if ( m_btld.isValid() ) {
        m_btld.powerOn();
        m_btld.setHostMode( QBluetoothLocalDevice::HostDiscoverable);
        QList<QBluetoothAddress> remotes;
        remotes = m_btld.connectedDevices();

        QBluetoothDeviceDiscoveryAgent btdda( this);
        connect( &btdda, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
                 this, SLOT(deviceDiscovery(QBluetoothDeviceInfo)));
        btdda.start();
    }
}

void BtCom::deviceDiscovery( const QBluetoothDeviceInfo& btdi)
{
    m_btdi.append( btdi);
    qDebug() << "BT device: " << btdi.name() << " (" << btdi.address().toString() << ")";
}
