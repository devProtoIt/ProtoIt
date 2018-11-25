#include "deviceviewer.h"
#include "ui_deviceviewer.h"
#include "mainwindow.h"
#include <QDir>
#include <QPushButton>


DeviceViewer::DeviceViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeviceViewer)
{
    ui->setupUi(this);
    m_main = (MainWindow*) parent;

    ui->tabViewer->setTabText( 0, tr( "Modellen"));
    ui->tabViewer->setTabText( 1, tr( "Apparaten"));
    ui->tabViewer->setTabText( 2, tr( "Functies"));

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->wwwViewer->settings()->setAttribute(QWebSettings::PluginsEnabled, true);

    ui->buttonBox->button( QDialogButtonBox::Ok)->setText( tr( "Voeg toe"));
    ui->buttonBox->button( QDialogButtonBox::Cancel)->setText( tr( "Annuleer"));

    listDevices();
}

DeviceViewer::~DeviceViewer()
{
    delete ui;
}

void DeviceViewer::listDevices()
{
    QDir dir;
    QString path = m_main->platformPath() + "Devices/";

    dir.setFilter( QDir::AllDirs | QDir::NoDotAndDotDot);

    dir.setPath( path + "Devices");
    m_devices = dir.entryList();
    ui->lstDevice->addItems( m_devices);

    dir.setPath( path + "Functions");
    m_functions = dir.entryList();
    ui->lstFunction->addItems( m_functions);

    dir.setPath( path + "Models");
    m_models = dir.entryList();
    ui->lstModel->addItems( m_models);
}

QStringList DeviceViewer::devices()
{
    QString path = m_main->platformPath() + "Devices/";
    QList<QListWidgetItem*> lwi;
    int i;

    m_selected.clear(); // in case this routine is called twices

    switch ( ui->tabViewer->currentIndex() ) {
        case 0 :
            lwi = ui->lstModel->selectedItems();
            for ( i = 0; i < lwi.count(); i++ )
                m_selected.append( path + "Models/" + lwi.at( i)->text() + ".pid");
            break;
        case 1 :
            lwi = ui->lstDevice->selectedItems();
            for ( i = 0; i < lwi.count(); i++ )
                m_selected.append( path + "Devices/" + lwi.at( i)->text() + ".pid");
            break;
        case 2 :
            lwi = ui->lstFunction->selectedItems();
            for ( i = 0; i < lwi.count(); i++ )
                m_selected.append( path + "Functions/" + lwi.at( i)->text() + ".pid");
            break;
    }

    return m_selected;
}

void DeviceViewer::on_lstModel_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString path = m_main->platformPath() + "Devices/Models/" +
                    m_models.at( ui->lstModel->currentRow()) + "/model.html";
    ui->wwwViewer->setUrl( QUrl::fromLocalFile( path));
}

void DeviceViewer::on_lstDevice_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString path = m_main->platformPath() + "Devices/Devices/" +
                    m_devices.at( ui->lstDevice->currentRow()) + "/model.html";
    ui->wwwViewer->setUrl( QUrl::fromLocalFile( path));
}

void DeviceViewer::on_lstFunction_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString path = m_main->platformPath() + "Devices/Functions/" +
                    m_functions.at( ui->lstFunction->currentRow()) + "/model.html";
    ui->wwwViewer->setUrl( QUrl::fromLocalFile( path));
}

void DeviceViewer::on_lstModel_itemDoubleClicked(QListWidgetItem *item)
{
    accept();
}

void DeviceViewer::on_lstDevice_itemDoubleClicked(QListWidgetItem *item)
{
    accept();
}

void DeviceViewer::on_lstFunction_itemDoubleClicked(QListWidgetItem *item)
{
    accept();
}
