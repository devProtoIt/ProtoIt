#include "serialselect.h"
#include "ui_serialselect.h"

SerialSelect::SerialSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SerialSelect)
{
    ui->setupUi(this);
}

SerialSelect::~SerialSelect()
{
    delete ui;
}

void SerialSelect::setComms( QStringList sl)
{
    for ( int i = 0; i < sl.count(); i++ )
        ui->lstComms->addItem( sl.at( i));
}

int SerialSelect::getComm()
{
    return m_port;
}

void SerialSelect::on_lstComms_currentRowChanged(int currentRow)
{
    m_port = currentRow;
}
