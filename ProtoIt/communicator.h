#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <QDialog>
#include <QStringList>
#include <QSerialPort>

namespace Ui {
class Communicator;
}

class Communicator : public QDialog
{
    Q_OBJECT

public:
    explicit Communicator(QWidget *parent = 0);
    ~Communicator();

    QString value();

private slots:

    void on_btnRight_released();
    void on_btnLeft_released();

private:
    Ui::Communicator *ui;

    void send( QString but);
    char m_test;
};

#endif // COMMUNICATOR_H
