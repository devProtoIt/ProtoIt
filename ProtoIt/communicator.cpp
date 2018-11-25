#include "communicator.h"
#include "ui_communicator.h"
#include <QtSerialPort/QtSerialPort>


////////////////////////////////////////////
////////////////////////////////////////////


Communicator::Communicator(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Communicator)
{
    ui->setupUi(this);

    ui->buttonBox->button( QDialogButtonBox::Ok)->setText( tr( "Voeg toe"));
    ui->buttonBox->button( QDialogButtonBox::Cancel)->setText( tr( "Annuleer"));

    m_test = 48;
}

Communicator::~Communicator()
{
    delete ui;
}

QString Communicator::value()
{
    return "(25),(138),(-12)";
}

void Communicator::send( QString but)
{
    int i;
    QList<QSerialPortInfo> ports;
    ports = QSerialPortInfo::availablePorts();
    for ( i = 0; i < ports.count(); i++ ) {
        QString line = ports.at( i).description().trimmed().toUpper();
        if ( line.left( 2) != "BT" ) break;
    }
    if ( i >= ports.count() ) {
        setWindowTitle( "Geen Arduino.");
        return;
    }

    QString port = ports.at( i).portName();


    QSerialPort serialPort;
    serialPort.setPortName(port);

    if ( !serialPort.open( QIODevice::ReadWrite) ) {
        setWindowTitle( "Geen poort geopend.");
        return;
    }

    if ( !serialPort.setBaudRate( 9600) ) {
        setWindowTitle( "Geen 9600 baud.");
        return;
    }

    if ( !serialPort.setDataBits( QSerialPort::Data8) ) {
        setWindowTitle( "Geen 8 data bits.");
        return;
    }

    if ( !serialPort.setParity( QSerialPort::NoParity) ) {
        setWindowTitle( "Geen parity-overeenkomst.");
        return;
    }

    if ( !serialPort.setStopBits( QSerialPort::OneStop) ) {
        setWindowTitle( "Geen stopbit.");
        return;
    }

    if ( !serialPort.setFlowControl( QSerialPort::NoFlowControl) ) {
        setWindowTitle( "Geen flow-control-overeenkomst.");
        return;
    }

    QByteArray ba = "12345\n";

    qint64 nByte = serialPort.write( ba);
    serialPort.flush();

    if (nByte == -1) {
        setWindowTitle( "Niets geschreven.");
    } else if (nByte != ba.count()) {
        setWindowTitle( "Niet alles geschreven.");
    } else if (!serialPort.waitForBytesWritten( 5000)) {
        setWindowTitle( "Time out.");
    }
    else {
        ba.clear();
        if ( serialPort.waitForReadyRead( 5000) )
            do {
                ba.append( serialPort.readAll());
            }
            while ( serialPort.waitForReadyRead( 20) );
        setWindowTitle( ba);
    }

    //setWindowTitle( "OK: " + QString::number( nByte) + "  bytes.");

    serialPort.close();
}

void Communicator::on_btnRight_released()
{
    if ( m_test < 57 )
        m_test++;
    else
        m_test = 48;
    send( "R");
}

void Communicator::on_btnLeft_released()
{
    if ( m_test > 48 )
        m_test--;
    else
        m_test = 58;
    send( "L");
}
