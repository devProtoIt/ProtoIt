// Copyright (C) 2014 D. Kruithof
//
// GNU General Public License Usage
// --------------------------------
// This file may be used under the terms of the GNU General Public
// License version 3.0 as published by the Free Software Foundation
// and appearing in the file 'GNU-GPL.txt' included in the packaging
// of this file.  Please review the following information to ensure
// the GNU General Public License version 3.0 requirements will be
// met: http://www.gnu.org/copyleft/gpl.html.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QListView>
#include <QMessageBox>
#include <QList>
#include <QSerialPort>
#include <QAction>
#include "step.h"
#include "tile.h"
#include "template.h"
#include "history.h"

namespace Ui {
class MainWindow;
}

typedef struct structVersion {
    QString     version;
    QString     platform;
    QString     programmer;
    QString     outputfile;
    QString     initvariant;
    QAction*    action;
    QStringList header;
    QStringList license;
    QStringList include;
    QStringList define;
    QStringList declaration;
    QStringList routine;
    QStringList setup;
    QStringList loop;
    QStringList maintask;
    QStringList board;
    QStringList compilemain;
    QStringList compilelibs;
    QStringList build;
    QStringList link;
    QStringList upload;
    QStringList message;
    QStringList serial;
    QStringList stoprobot;
} Version;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setTitle();
    void setMenu();

    int message( QMessageBox::Icon icon, QString info, QString ask = "");

    bool serialOpen();
    void serialClose();
    bool serialWrite( QString data);
    QStringList serialRead();
    QSerialPort* serialPort();

    void adjustScroll();

    QString homePath();         // path to the folder with the executable
    QString pluginPath();       // path to the folder containing the subdir 'versions', 'platforms' and 'programmers'
    QString versionFile();      // path to the version configuration file
    QString platformPath();     // path to the active platform
    QString programmerPath();   // path to the active programmer
    QString buildPath();        // path to the build directory
    QString projectPath();      // path to the saved project

    QTabWidget* getTab();
    QToolBar* getTool();

    QStringList loadVersions();
    int  indexOfVersion( QString version);
    void setVersion( int current, bool uncheck = true);
    QString version();
    QString initVariant();      // template for Variant usage
    QString platform();
    void lockPlatform();
    void freePlatform();

    void unselectSignal();
    QString findSignalKey( QString signal);
    QString findStepNumber(QString signal);

    bool isDeviceUsed( QString alias);

    void setSignals( Template* tmpl);
    void setConstants( Template* tmpl);
    void createTool( Template* tmpl);
    void deleteToolSignals( Template* tmpl);

    Step* addStep();
    Step* setStep( int index);
    int   setStep( Step* step);
    void  removeStep( int index);
    Step* insertStep( Step* step, int index, QString alias = "");
    void  moveStep( int ixfrom, int ixto);

    Tile* createDevice( QString filepath, QString device = "", QString alias = "");
    void  insertDevice( Template* templ, int index);
    void  removeDevice( QString alias);
    Tile* createTile( int stepix, QString alias);

protected:

    void resizeEvent(QResizeEvent* qre);
    void closeEvent( QCloseEvent *event);

private slots:

    void on_stepTab_currentChanged(int index);
    void on_scrollTab_valueChanged(int value);

    void on_adddevice_triggered();
    void on_actuatordevice_triggered();
    void on_signaldevice_triggered();

    void on_actionNew_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionOpen_triggered();
    void on_actionUpload_triggered();
    void on_actionExport_triggered();
    void on_actionPlatform_triggered();
    void on_actionPlatformManager_triggered();
    void on_actionQuit_triggered();

    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionReport_triggered();

    void on_actionHelp_triggered();
    void on_actionAbout_triggered();

    void on_actionDatalogger_triggered();
    void on_actionStop_triggered();
    void on_actionSetTime_triggered();
    void on_actionMonitor_triggered();

    void on_butDelete_clicked();
    void on_butRename_clicked();
    void on_butToBack_clicked();
    void on_butToFront_clicked();

private:
    Ui::MainWindow *ui;

    QString             NEWPROJECT;

    QList<Template*>    m_templates;
    QStringList         m_signals;
    QStringListModel    m_smodel;
    QStringList         m_constants;
    QStringListModel    m_cmodel;

    QString             m_tooltext;

    QList<Version*>     m_versions;
    int                 m_curversion;
    Version*            m_version;
    QString             m_platform;
    QString             m_project;

    QString             m_progpath; // path to the folder containing the versions, platforms and programmers

    bool                m_silent;   // don't display messages
    bool                m_movestep; // in the process of moving a step
    int                 m_curstep;
    Step*               m_addstep;

    QSerialPort         m_serial;
    QString             m_serialport;

    void resizeWnd();
    void writeCode();
    void uploadCode();
    QString replaceParam( QString cmd, QString param, QStringList& items);
    QString replaceParam( QString cmd, QString param, QString item);
    void renumberSteps();
    bool checkSave(); // returns false if canceled
    bool askSave();
    void doSave();
    void doNew();
    void doOpen( QString project);
    void saveDevice( QString srcpath, QString dstpath, QString device);
    QString findSerialPort();
    bool addVersion( QString path, QString platform);
};

#endif // MAINWINDOW_H
