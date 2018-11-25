#include "historyreport.h"
#include "ui_historyreport.h"

HistoryReport::HistoryReport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryReport)
{
    ui->setupUi(this);
}

HistoryReport::~HistoryReport()
{
    delete ui;
}

void HistoryReport::addRow( QString row)
{
    ui->lstReport->addItem( row);
}
