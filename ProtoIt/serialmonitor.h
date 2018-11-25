#ifndef SERIALMONITOR_H
#define SERIALMONITOR_H

#include <QDialog>
#include <QListWidget>
#include "mainwindow.h"

namespace Ui {
class SerialMonitor;
}

class SerialMonitor : public QDialog
{
    Q_OBJECT

public:
    explicit SerialMonitor(QWidget *parent = 0);
    ~SerialMonitor();

private slots:
    void on_btnPause_clicked();
    void on_btnClear_clicked();
    void readData();

    void on_hslTempo_valueChanged(int value);

private:
    Ui::SerialMonitor *ui;

    MainWindow* m_main;
    bool        m_pause;
    QString     m_text;
    int         m_delay;
    int         m_count;
};

#endif // SERIALMONITOR_H
