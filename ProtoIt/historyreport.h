#ifndef HISTORYREPORT_H
#define HISTORYREPORT_H

#include <QDialog>

namespace Ui {
class HistoryReport;
}

class HistoryReport : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryReport(QWidget *parent = 0);
    ~HistoryReport();

    void addRow( QString row);

private:
    Ui::HistoryReport *ui;
};

#endif // HISTORYREPORT_H
