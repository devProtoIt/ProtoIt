#ifndef SERIALSELECT_H
#define SERIALSELECT_H

#include <QDialog>

namespace Ui {
class SerialSelect;
}

class SerialSelect : public QDialog
{
    Q_OBJECT

public:
    explicit SerialSelect(QWidget *parent = 0);
    ~SerialSelect();

    void setComms( QStringList sl);
    int getComm();

private slots:
    void on_lstComms_currentRowChanged(int currentRow);

private:
    Ui::SerialSelect *ui;

    int m_port;
};

#endif // SERIALSELECT_H
