#ifndef PLATFORMMANAGER_H
#define PLATFORMMANAGER_H

#include <QDialog>
#include <QListWidgetItem>
#include "mainwindow.h"

namespace Ui {
class PlatformManager;
}

class PlatformManager : public QDialog
{
    Q_OBJECT

public:
    explicit PlatformManager(QWidget *parent);
    ~PlatformManager();

    static QStringList listDirs( QString path);

private slots:
    void on_lstVersionOld_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_lstVersionNew_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_butGetNewVersions_clicked();
    void on_butDelVersion_clicked();
    void on_butDelDevice_clicked();
    void on_butAddVersion_clicked();
    void on_butAddDevice_clicked();

private:

    void readConfig( QString& version, QString& platform, QString& programmer, QString versionfile);
    void listVersions( int type, QString path);
    void listDevices( QListWidget* list, QString path);
    void reportDeletions();
    void reportInstallations();

    void deleteType( int type, QString path);
    void copyType( int type, QString srcpath, QString dstpath);
//    bool deleteRecursively( QString path); // identical to QDir::removeRecursively
    bool copyRecursively( QString srcpath, QString dstpath, bool overwrite);

    Ui::PlatformManager *ui;
    MainWindow* m_main;

    QStringList m_versionOld;
    QStringList m_platformOld;
    QStringList m_progrOld;
    int         m_curVersionOld;
    int         m_curPlatformOld;
    int         m_curProgrOld;

    QStringList m_versionNew;
    QStringList m_platformNew;
    QStringList m_progrNew;
    int         m_curVersionNew;
    int         m_curPlatformNew;
    int         m_curProgrNew;

    QList<int>  m_devTypeNew; // reflects the entries in ui->lstDeviceNew
    QList<int>  m_devTypeOld; // reflects the entries in ui->lstDeviceOld

    QString     m_pathOld; // equals to m_main->pluginPath()
    QString     m_pathNew;

    QStringList m_updates;
};

#endif // PLATFORMMANAGER_H
