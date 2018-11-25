#ifndef DEVICEVIEWER_H
#define DEVICEVIEWER_H

#include <QDialog>
#include <QListWidgetItem>

class MainWindow;

namespace Ui {
class DeviceViewer;
}

class DeviceViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceViewer(QWidget *parent = 0);
    ~DeviceViewer();

    QStringList devices();

private:
    Ui::DeviceViewer *ui;

    void listDevices();

private slots:
    void on_lstModel_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_lstDevice_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_lstFunction_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_lstModel_itemDoubleClicked(QListWidgetItem *item);
    void on_lstDevice_itemDoubleClicked(QListWidgetItem *item);
    void on_lstFunction_itemDoubleClicked(QListWidgetItem *item);

protected:

    MainWindow*     m_main;
    QStringList     m_models;
    QStringList     m_devices;
    QStringList     m_functions;
    QStringList     m_selected;
};

#endif // DEVICEVIEWER_H
