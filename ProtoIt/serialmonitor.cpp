#include "serialmonitor.h"
#include "ui_serialmonitor.h"
#include <QThread>
#include <QTime>

SerialMonitor::SerialMonitor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialMonitor)
{
    ui->setupUi(this);
    ui->hslTempo->setRange( 0, 500);
    ui->hslTempo->setValue( 480);
    m_pause = true;
    m_delay = 20;
    m_count = 0;
    m_main = (MainWindow*) parent;
    connect( m_main->serialPort(), SIGNAL( readyRead()), this, SLOT( readData()));
}

SerialMonitor::~SerialMonitor()
{
    if ( !m_pause )
        m_main->serialWrite( "RTCMDMON");
    delete ui;
}

void SerialMonitor::readData()
{
    m_count++;
    QByteArray ba = m_main->serialPort()->readAll();
    if ( !m_pause ) {
        for ( int i = 0; i < ba.count(); i++ ) {
            if ( ba[i] != '\n' )
                m_text += ba[i];
            else {
                if ( m_count > m_delay ) {
                    ui->txbMonitor->append( m_text);
                    m_count = 0;
                }
                m_text = "";
            }
        }
    }
}

void SerialMonitor::on_btnPause_clicked()
{
    m_pause = !m_pause;
    ui->btnPause->setText( m_pause ? tr( "Start") : tr( "Stop"));
    m_main->serialWrite( "RTCMDMON");
}

void SerialMonitor::on_btnClear_clicked()
{
    ui->txbMonitor->clear();
}

void SerialMonitor::on_hslTempo_valueChanged(int value)
{
    m_delay = 500 - value;
}
