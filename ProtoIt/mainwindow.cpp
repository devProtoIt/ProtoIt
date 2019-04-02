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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QResizeEvent>
#include <QtSerialPort/QtSerialPort>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QByteArray>
#include <QProcess>
#include <QComboBox>
#include <QPushButton>
#include <QDrag>
#include <QMimeData>
#include "tileid.h"
#include "info.h"
#include "qplatform.h"
#include "renamestep.h"
#include "helpviewer.h"
#include "deviceviewer.h"
#include "serialmonitor.h"
#include "serialselect.h"
#include "platformmanager.h"

QString tabstyle =
        "QTabWidget::pane { "
        "border-top: 2px solid #D8D8D8; "
        "} "
        "QTabBar::tab { "
        "background: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 #E1E1E1, stop: 0.4 #DDDDDD, stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3); "
        "border: 2px solid #C4C4C3; "
        "border-bottom-color: #C2C7CB; "
        "border-top-left-radius: 4px; "
        "border-top-right-radius: 8px; "
        "min-width: 8px; "
        "padding: 2px; "
        "} "
        "QTabWidget::tab-bar { "
        "left: 5px; "
        "} "
        "QTabBar::tab:selected, QTabBar::tab:hover { "
        "background: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1,"
        "stop: 0 #FAFAFA, stop: 0.4 #F4F4F4, stop: 0.5 #E7E7E7, stop: 1.0 #FAFAFA)"
        "} "
        "QTabBar::tab:selected { "
        "border-color: #9B9B9B; "
        "border-bottom-color: #C2C7C8; "
        "} "
        "QTabBar::tab:!selected { "
        "margin-top: 2px;"
        "} ";

QString toMsDosName( const QString path, const QString name)
{
    if ( name.indexOf( ' ') < 0 )
        return name;

    // prepare msdos name
    int cnt = 0, ix;
    QString ext, nam;
    if ( (ix = name.lastIndexOf( '.')) < 0 )
        nam = name;
    else {
        nam = name.left( ix);
        ext = name.right( name.length() - ix);
    }
    for ( ix = 0; ix < nam.length(); ix++ ) {
        if ( nam.at( ix) == ' ' )
            cnt++;
        if ( ix - cnt == 6 )
            break; // this is the maximum msdos name length when a space is contained
    }

    // find entry number within all matching msdos names
    QDir dir( path);
    QStringList flt;
    flt.append( nam.left( ix) + "*");
    QStringList sub = dir.entryList( flt);
    ix = 0;
    foreach ( QString s, sub ) {
        ix++;
        if ( s == nam + ext )
            break;
    }

    // create msdos name
    nam.remove( ' ');
    nam = nam.left( 6).toUpper() + "~" + QString::number( ix) + ext;
    return nam;
}

QString toMsDos( const QString path)
{
    QStringList lst = path.split( '/');
    QString nam, pth = lst.at( 0);
    for ( int ix = 1; ix < lst.count(); ix++ ) {
        nam = lst.at( ix);
        lst.replace( ix, toMsDosName( pth, nam));
        pth += "/" + nam;
    }
    pth = lst.at( 0);
    lst.removeFirst();
    foreach ( nam, lst )
        pth += "/" + nam;
    return pth;
}


////////////////////////////////////////
/// MAINWINDOW
////////////////////////////////////////


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    NEWPROJECT = "<" + tr( "Nieuw") + ">";

    g_history.setMain( this);

    ui->centralWidget->setStyleSheet( "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 0, 0, 255), stop:1 rgba(255, 255, 255, 255));color:blue;");

    QAction* action;
    action = new QAction( ui->toolBar);
    action->setIcon( QIcon( ":/upload.png"));
    ui->toolBar->addAction( action);
    ui->toolBar->connect( action, SIGNAL(triggered()), this, SLOT(on_actionUpload_triggered()));
    action = new QAction( ui->toolBar);
    action->setIcon( QIcon( ":/addtile.png"));
    ui->toolBar->addAction( action);
    ui->toolBar->connect( action, SIGNAL(triggered()), this, SLOT(on_adddevice_triggered()));

    m_smodel.setStringList( m_signals);
    ui->listSignals->setModel( &m_smodel);
    m_cmodel.setStringList( m_constants);
    ui->listConstants->setModel( &m_cmodel);

    setStyleSheet( tabstyle);
    m_addstep = new Step( this);
    ui->stepTab->addTab( m_addstep, ""); // tab 'new step'
    // adding this tab triggers an event to add a new tab "Instellingen" too
    // so the tab index becomes 1 instead of 0
    ui->stepTab->setTabIcon( 1, QIcon( ":/addtab.png"));
    ui->stepTab->setCurrentIndex( 0);
    ((Step*)ui->stepTab->widget( 0))->setIndex( 0);

    QDir::setCurrent( homePath()); // only needed for testing

    ui->listSignals->setDragEnabled( true);
    ui->listConstants->setDragEnabled( true);

    QStringList args = QCoreApplication::arguments();
    if ( args.count() > 1 )
        doOpen( args.at( 1));
    if ( !m_project.isEmpty() )
        return;

    m_silent = false;
    m_movestep = false;
    m_curstep = 0;
    m_project = NEWPROJECT;

    setTitle();

    m_progpath = homePath();

    m_version = NULL;
    QStringList versions = loadVersions();

    QPlatform pf;
    pf.setPlatforms( m_progpath + "versions/", versions);
    pf.exec();

    setVersion ( pf.selectedPlatform(), false);

    setMenu();
}

MainWindow::~MainWindow()
{
    for ( int i = 0; i < m_templates.count(); i++ )
        delete m_templates.at( i);
    m_templates.clear();
    delete ui;
}

void MainWindow::resizeWnd()
{
    int sx = ui->centralWidget->width();
    int sy = ui->centralWidget->height();
    int dy;

    if ( sx > 295 ) {
        // show the tabs and scroll bar when there is enough space
        ui->butDelete->setHidden( false);
        ui->butRename->setHidden( false);
        ui->scrollTab->setHidden( false);
        ui->stepTab->setHidden( false);

        ui->stepTab->resize( sx - ui->labelS->width() - 50, sy - 20);
        ui->stepTab->move( ui->labelS->width() + 20, 10);

        int butheight = ui->butDelete->height() + 1;
        ui->scrollTab->resize( ui->scrollTab->width(), sy - 4 * butheight - 50);
        ui->butDelete->move( sx - 30, 38);
        ui->butRename->move( sx - 30, butheight + 38);
        ui->butToFront->move( sx - 30, 2 * butheight + 38);
        ui->butToBack->move( sx - 30, 3 * butheight + 38);
        ui->scrollTab->move( sx - 30, 4 * butheight + 38);

        dy = (sy - 2 * ui->labelS->height() - 20) / 2;
        ui->listSignals->resize( ui->listSignals->width(), dy - 10);
        ui->listConstants->resize( ui->listConstants->width(), dy - 10);
        ui->listSignals->move( 10, ui->labelS->height() + 15);
        ui->labelC->move( 10, sy / 2 + 5);
        ui->listConstants->move( 10, sy / 2 + ui->labelC->height() + 10);

        QSize sz = ui->stepTab->currentWidget()->size();
        for ( int i = 0; i < ui->stepTab->count() - 1; i ++ ) {
            ui->stepTab->widget( i)->resize( sz); // force a resize on all tabs
            ((Step*) ui->stepTab->widget( i))->modelAllTiles();
        }
    }
    else {
        // hide the tabs and scroll bars when there is enough space
        ui->butDelete->setHidden( true);
        ui->butRename->setHidden( true);
        ui->scrollTab->setHidden( true);
        ui->stepTab->setHidden( true);
    }
}

void MainWindow::resizeEvent(QResizeEvent* qre)
{
    resizeWnd();
    qre->accept();
}

void MainWindow::closeEvent( QCloseEvent *event)
{
    if ( checkSave() )
        event->accept(); // = close
    else
        event->ignore(); // = don't close
}

void MainWindow::setTitle()
{
    setWindowTitle( "ProtoIt : " + m_project);
}

void MainWindow::setMenu()
{
    if ( !m_version ) return;

    if ( m_version->serial.isEmpty() && m_version->stoprobot.isEmpty() ) {
        ui->menuRobot->setEnabled( false);
        return;
    }

    ui->menuRobot->setEnabled( true);
    ui->actionStop->setEnabled( m_version->stoprobot.count());

    if ( m_version->serial.count() && (m_version->serial.at( 0) == "YES") ) {
        ui->actionSetTime->setEnabled( true);
        ui->actionDatalogger->setEnabled( true);
        ui->actionMonitor->setEnabled( true);
    }
    else {
        ui->actionSetTime->setEnabled( false);
        ui->actionDatalogger->setEnabled( false);
        ui->actionMonitor->setEnabled( false);
    }
}

int MainWindow::message( QMessageBox::Icon icon, QString info, QString ask)
{
    if ( m_silent ) return 0;

    QMessageBox msg;
    msg.setWindowTitle( "ProtoIt");
    msg.setStyleSheet( "background:lightgray");
    msg.setText( info);
    msg.setInformativeText( ask);
    msg.setIcon( icon);
    if ( ask.isEmpty() ) {
        msg.setStandardButtons( QMessageBox::Ok);
        msg.setDefaultButton( QMessageBox::Ok);
    }
    else {
        msg.setStandardButtons( QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msg.setDefaultButton( QMessageBox::Cancel);
        msg.button( QMessageBox::Yes)->setText( tr( "Ja"));
        msg.button( QMessageBox::No)->setText( tr( "Nee"));
        msg.button( QMessageBox::Cancel)->setText( tr( "Annuleer"));
    }
    return msg.exec();
}

QStringList MainWindow::loadVersions()
{
    int i;
    for ( i = 0; i < m_versions.count(); i++ ) {
        m_versions.at( i)->action->disconnect();
        ui->menuPlatform->removeAction( m_versions.at( i)->action);
        delete m_versions.at( i);
    }
    m_versions.clear();

    QStringList versions = PlatformManager::listDirs( m_progpath + "versions/");
    i = 0;
    while ( i < versions.count() ) {
        if ( !addVersion( m_progpath, versions.at( i)) )
            versions.removeAt( i);
        else
            i++;
    }

    for ( i = 0; i < m_versions.count(); i++ ) {
        m_versions.at( i)->action = ui->menuPlatform->addAction( m_versions.at( i)->version);
        m_versions.at( i)->action->setCheckable( true);
        connect( m_versions.at( i)->action, SIGNAL(triggered()), this, SLOT( on_actionPlatform_triggered()));
    }

    m_curversion = 0;
    return versions;
}

int MainWindow::indexOfVersion( QString version)
{
    for ( int i = 0; i < m_versions.count(); i++ )
        if ( m_versions.at( i)->version == version )
            return i;
    return -1;
}

void MainWindow::setVersion( int ix, bool uncheck)
{
    if ( ix < 0 || ix >= m_versions.count() ) return;

    if ( m_version && uncheck ) m_version->action->setChecked( false);

    m_version = m_versions.at( ix);
    m_version->action->setChecked( true);
    m_platform = m_version->platform;

    m_curversion = ix;
}

QString MainWindow::version()
{
    if ( m_version )
        return m_version->version;
    else
        return "";
}

QString MainWindow::initVariant()
{
    if ( m_version )
        return m_version->initvariant;
    else
        return "";
}

bool MainWindow::addVersion( QString path, QString version)
{
    QFile file( path + "versions/" + version + "/" + version + ".cfg");
    QTextStream in( &file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        message( QMessageBox::Information, tr( "Platform '") + version + tr( "' werd niet gevonden.\nInstalleer ProtoIt opnieuw."));
        return false;
    }

    QString line, upper;
    QStringList* list = NULL;
    Version* vn = new Version;

    while ( !in.atEnd() ) {
        line = in.readLine();
        upper = line.trimmed().toUpper();
        if ( upper.isEmpty() ) continue;

        if ( upper == "[PLATFORM]" ) {
            while ( !in.atEnd() ) {
                line = in.readLine().trimmed();
                if ( line.isEmpty() ) continue;
                if ( line.left(1) != "[" ) {
                    vn->platform = line;
                     break;
                }
            }
            if ( vn->platform.isEmpty() ) {
                message( QMessageBox::Information, tr( "Platform voor versie '") + version + tr( "' onbekend.\nInstalleer ProtoIt opnieuw."));
                file.close();
                return false;
            }
        }
        else
        if ( upper == "[VERSION]" ) {
            while ( !in.atEnd() ) {
                line = in.readLine().trimmed();
                if ( line.isEmpty() ) continue;
                if ( line.left(1) != "[" ) {
                    vn->version = line;
                    break;
                }
            }
            if ( vn->version.isEmpty() ) {
                message( QMessageBox::Information, tr( "Versie '") + version + tr( "' onbekend.\nInstalleer ProtoIt opnieuw."));
                file.close();
                return false;
            }
        }
        else
        if ( upper == "[PROGRAMMER]" ) {
            while ( !in.atEnd() ) {
                line = in.readLine().trimmed();
                if ( line.isEmpty() ) continue;
                if ( line.left(1) != "[" ) {
                    vn->programmer = line;
                    break;
                }
            }
            if ( vn->programmer.isEmpty() ) {
                message( QMessageBox::Information, tr( "Programmer voor versie '") + version + tr( "' onbekend.\nInstalleer ProtoIt opnieuw."));
                file.close();
                return false;
            }
        }
        else
        if ( upper == "[OUTPUTFILE]" ) {
            while ( !in.atEnd() ) {
                line = in.readLine().trimmed();
                if ( line.isEmpty() ) continue;
                if ( line.left(1) != "[" ) {
                    vn->outputfile = line;
                    break;
                }
            }
            if ( vn->outputfile.isEmpty() ) {
                message( QMessageBox::Information, tr( "Output-bestand voor versie '") + version + tr( "' onbekend.\nInstalleer ProtoIt opnieuw."));
                file.close();
                return false;
            }
        }
        else
        if ( upper == "[INITVARIANT]" ) {
            while ( !in.atEnd() ) {
                line = in.readLine().trimmed();
                if ( line.isEmpty() ) continue;
                if ( line.left(1) != "[" ) {
                    vn->initvariant = line;
                    break;
                }
            }
            if ( vn->initvariant.isEmpty() ) {
                message( QMessageBox::Information, tr( "Output-bestand voor versie '") + version + tr( "' onbekend.\nInstalleer ProtoIt opnieuw."));
                file.close();
                return false;
            }
        }
        else
        if ( upper == "[HEADER]" )
            list = &vn->header;
        else
        if ( upper == "[LICENSE]" )
            list = &vn->license;
        else
        if ( upper == "[INCLUDE]" )
            list = &vn->include;
        else
        if ( upper == "[DEFINE]" )
            list = &vn->define;
        else
        if ( upper == "[DECLARATION]" )
            list = &vn->declaration;
        else
        if ( upper == "[ROUTINE]" )
            list = &vn->routine;
        else
        if ( upper == "[SETUP]" )
            list = &vn->setup;
        else
        if ( upper == "[LOOP]" )
            list = &vn->loop;
        else
        if ( upper == "[MAIN]" )
            list = &vn->maintask;
        else
        if ( upper == "[BOARD]" )
            list = &vn->board;
        else
        if ( upper == "[COMPILEMAIN]" )
            list = &vn->compilemain;
        else
        if ( upper == "[COMPILELIBS]" )
            list = &vn->compilelibs;
        else
        if ( upper == "[BUILD]" )
            list = &vn->build;
        else
        if ( upper == "[LINK]" )
            list = &vn->link;
        else
        if ( upper == "[UPLOAD]" )
            list = &vn->upload;
        else
        if ( upper == "[MESSAGE]" )
            list = &vn->message;
        else
        if ( upper == "[SERIAL]" ) {
            list = &vn->serial;
        }
        else
        if ( upper == "[STOPROBOT]" ) {
            list = &vn->stoprobot;
        }
        else
            if ( !list || line.left( 1) == "[" ) {
                message( QMessageBox::Information, tr( "Platform '") + version + tr( "' is verminkt.\nInstalleer ProtoIt opnieuw."));
                return false;
            }
            else
                list->append( line);
    }
    file.close();

    m_versions.append( vn);
    return true;
}

QString MainWindow::findSerialPort()
{
    QList<QSerialPortInfo> ports;
    ports = QSerialPortInfo::availablePorts();

    if ( !ports.count() ) return "";
    if ( !m_version ) return "";

    int ib, ip;
    QString board;
    for ( ib = 0; ib < m_version->board.count(); ib++ ) {
        board = m_version->board.at( ib).toUpper();
        for ( ip = 0; ip < ports.count(); ip++ ) {
            QString p = ports.at( ip).description().trimmed();
            if ( ports.at( ip).description().trimmed().toUpper() == board ) break;
        }
        if ( ip < ports.count() ) break;
    }
    if ( ib >= m_version->board.count() ) return "";

    return ports.at( ip).portName();
}

bool MainWindow::serialOpen()
{
    if ( m_serial.isOpen() ) return true;
    if ( !m_version ) return false;

    m_serialport = findSerialPort();
    if ( m_serialport.isEmpty() ) return false;

    QString dummy;
    m_serial.setPortName( m_serialport);
    if ( !m_serial.open( QIODevice::ReadWrite) ) {
        // baud 9600, 8 data bits, no parity, one stop bit, no flow control
        message( QMessageBox::Warning, tr( "Seriële poort is niet beschikbaar."));
        return false;
    }
    QThread::sleep( 1); // nodig voordat er kan worden geschreven of gelezen

    return true;
}

void MainWindow::serialClose()
{
    if ( !m_serial.isOpen() ) return;
    m_serial.close();
}

bool MainWindow::serialWrite( QString data)
{
    if ( !m_serial.isOpen() ) goto serialfailure;

    {
    QByteArray ba = data.toLocal8Bit() + '\n';
    m_serial.write( ba);
    m_serial.flush();
    if ( !m_serial.waitForBytesWritten( 5000) )
        goto serialfailure;
    }
    return true;

serialfailure:

    message( QMessageBox::Information, tr( "Er kon geen data worden verstuurd."));
    return false;
}

QStringList MainWindow::serialRead()
{
    QStringList sl;
    if ( !m_serial.isOpen() ) return sl;

    QByteArray ba;
    QString data;
    if ( m_serial.waitForReadyRead( 5000) ) {
        do {
            ba = m_serial.readAll();
            for ( int i = 0; i < ba.count(); i++ ) {
                if ( ba[i] == '\n' ) {
                    sl.append( data);
                    data = "";
                }
                else
                    data += ba[i];
            }
            ba.clear();
        }
        while ( m_serial.waitForReadyRead( 1000) );
        // after one second end of communication is supposed
        if ( !data.isEmpty() ) sl.append( data);
    }
    else
        goto serialfailure;

    return sl;

serialfailure:

    message( QMessageBox::Information, tr( "Er werd geen data ontvangen."));
    return sl;
}

QSerialPort* MainWindow::serialPort()
{
    return &m_serial;
}

QTabWidget* MainWindow::getTab()
{
    return ui->stepTab;
}

QToolBar* MainWindow::getTool()
{
    return ui->toolBar;
}

void MainWindow::lockPlatform()
{
    for ( int i = 0; i < m_versions.count(); i++ )
        m_versions.at( i)->action->setEnabled( false);
    ui->actionPlatformManager->setEnabled( false);
}

void MainWindow::freePlatform()
{
    for ( int i = 0; i < m_versions.count(); i++ )
        m_versions.at( i)->action->setEnabled( true);
    ui->actionPlatformManager->setEnabled( true);
}

QString MainWindow::homePath()
{
    return QCoreApplication::applicationDirPath() + "/";
}

QString MainWindow::pluginPath()
{
    return m_progpath;
}

QString MainWindow::versionFile()
{
    if ( m_curversion < 0 || m_curversion >= m_versions.count() )
        return "";
    return m_progpath + "versions/" + m_versions.at( m_curversion)->platform + '/' +
                                       m_versions.at( m_curversion)->platform + '.cfg';
}

QString MainWindow::platformPath()
{
    if ( m_curversion < 0 || m_curversion >= m_versions.count() )
        return "";
    return m_progpath + "platforms/" + m_versions.at( m_curversion)->platform + '/';
}

QString MainWindow::programmerPath()
{
    if ( m_curversion < 0 || m_curversion >= m_versions.count() )
        return "";
    return m_progpath + "programmers/" + m_versions.at( m_curversion)->programmer + '/';
}

QString MainWindow::buildPath()
{
    QDir curdir = QDir::temp();
    curdir.mkdir( "ProtoItBuild");
    return QDir::tempPath() + "/ProtoItBuild/";
}

QString MainWindow::projectPath()
{
    return m_project.left( m_project.lastIndexOf( '.')) + '/';
}

QString MainWindow::platform()
{
    return m_platform;
}

void MainWindow::adjustScroll()
{
    Step *step = (Step*) ui->stepTab->currentWidget();
    ui->scrollTab->setRange( 0, step->sizeY() > ui->scrollTab->height() ? step->sizeY() - ui->scrollTab->height() : 0);
}

QString MainWindow::findSignalKey( QString signal)
{
    Step* step = (Step*) ui->stepTab->widget( 0);

    QString key;
    Tile* tile = step->firstTile();
    while ( tile ) {
        key = tile->findSignalKey( signal);
        if ( !key.isEmpty() )
            return key;
        tile = step->nextTile();
    }
    return "";
}

QString MainWindow::findStepNumber( QString signal){
    for ( int i = 1; i < ui->stepTab->count(); i++ )
        if ( signal == ui->stepTab->tabText( i) )
            return QString::number( i);
    return "";
}

void MainWindow::setSignals( Template* tmpl)
{
    Var* var;
    m_signals.clear();
    var = tmpl->m_vars.firstVar( vtSignalOut);
    while ( var ) {
        if ( var->vtab ) {
            if ( !m_signals.isEmpty() )
                m_signals.append( "");
            if ( !var->paragraph.isEmpty() )
                m_signals.append( var->paragraph);
        }
        m_signals.append( tmpl->alias() + " > " + var->alias);
        var = tmpl->m_vars.nextVar( vtSignalOut);
    }
    m_smodel.setStringList( m_signals);
}

void MainWindow::setConstants( Template* tmpl)
{
    Var* var;
    m_constants.clear();
    var = tmpl->m_vars.firstVar( vtConstant);
    while ( var ) {
        if ( var->vtab ) {
            if ( !m_constants.isEmpty() )
                m_constants.append( "");
            if ( !var->paragraph.isEmpty() )
                m_constants.append( var->paragraph);
        }
        m_constants.append( tmpl->alias() + " > " + var->alias);
        var = tmpl->m_vars.nextVar( vtConstant);
    }
    m_cmodel.setStringList( m_constants);
}

void MainWindow::createTool( Template* tmpl)
{
    // create device tool on the toolbars

    if ( tmpl->m_vars.firstVar( vtSignalIn) ) {
        QAction* tool = ui->actuatorBar->addAction( QIcon( tmpl->icon()), tmpl->alias());
        ui->actuatorBar->connect( tool, SIGNAL(triggered()), this, SLOT(on_actuatordevice_triggered()));
    }

    if ( tmpl->m_vars.firstVar( VarType( vtSignalOut | vtConstant )) ) {
        QAction* tool = ui->sensorBar->addAction( QIcon( tmpl->icon()), tmpl->alias());
        ui->sensorBar->connect( tool, SIGNAL(triggered()), this, SLOT(on_signaldevice_triggered()));
    }

    // a less nice way to have resizeStep() working properly
    // (somehow it doesn't when invoking directly)
    resize( width(), height()-1); resize( width(), height()+1);
}

void MainWindow::deleteToolSignals( Template* tmpl)
{
    // remove signals from signal and constant list

    QString signal;
    Tile* tile = ((Step*) ui->stepTab->widget( 0))->tile( tmpl->alias());
    Field* field = tile->firstField();
    while ( field ) {
        signal = tmpl->alias() + " > " + field->alias();
        if ( m_signals.indexOf( signal) >= 0 )
            m_signals.clear();
        if ( m_constants.indexOf( signal) >= 0 )
            m_constants.clear();
        field = tile->nextField();
    }
    m_smodel.setStringList( m_signals);
    m_cmodel.setStringList( m_constants);

    // remove device tool from actuatorbar

    QList<QAction*> tl;
    tl = ui->actuatorBar->actions();
    for ( int i = 0; i < tl.count(); i++ ) {
        QAction* tool = tl.at( i);
        if ( tool->iconText() == tmpl->alias() ) {
            ui->actuatorBar->disconnect( tool, 0, 0, 0);
            ui->actuatorBar->removeAction( tool);
            delete tool;
            break;
        }
    }

    // remove device tool from sensobar

    tl = ui->sensorBar->actions();
    for ( int i = 0; i < tl.count(); i++ ) {
        QAction* tool = tl.at( i);
        if ( tool->iconText() == tmpl->alias() ) {
            ui->sensorBar->disconnect( tool, 0, 0, 0);
            ui->sensorBar->removeAction( tool);
            delete tool;
            break;
        }
    }
    resize( width(), height()-1); resize( width(), height()+1);
}

bool MainWindow::isDeviceUsed( QString alias)
{
    Step* step;
    Tile* tile;

    for ( int i = 1; i < ui->stepTab->count(); i++ ) {
        step = (Step*) ui->stepTab->widget( i);
        tile = step->firstTile();
        while ( tile ) {
            if ( tile->alias() == alias ) {
                message( QMessageBox::Information, tr( "Het apparaat is nog onderdeel van een stap.\nVerwijder het apparaat eerst van alle stappen."));
                return true;
            }
            if ( tile->isDeviceUsed( alias) ) {
                message( QMessageBox::Information, tr( "Signalen van dit apparaat zijn nog in gebruik.\nMaak dit eerst ongedaan."));
                return true;
            }
            tile = step->nextTile();
        }
    }
    return false;
}

Step* MainWindow::addStep()
{
    return m_addstep;
}

Step* MainWindow::setStep( int index)
{
    ui->stepTab->setCurrentIndex( index);
    return (Step*) ui->stepTab->currentWidget();
}

int MainWindow::setStep( Step* step)
{
    ui->stepTab->setCurrentWidget( step);
    return ui->stepTab->currentIndex();
}

void MainWindow::removeStep( int index)
{
    int ix = ui->stepTab->currentIndex();
    ui->stepTab->setCurrentIndex( 0); // cannot remove an active tab
    ui->stepTab->removeTab( index);
    if ( ui->stepTab->count() > ix + 1 )
        ui->stepTab->setCurrentIndex( ix);
    else if ( ix > 1 )
        ui->stepTab->setCurrentIndex( ix - 1);
    renumberSteps();
}

Step* MainWindow::insertStep( Step* step, int index, QString alias)
{
    ui->stepTab->insertTab( index, step, alias);
    ui->stepTab->setCurrentIndex( index);
    renumberSteps();
    return step;
}

void MainWindow::renumberSteps()
{
    QList<int> oldix;
    QList<int> newix;
    Step *step;

    // renumber the tabs
    QString steptxt = tr( "Stap");
    for ( int i = 1; i < ui->stepTab->count() - 1; i++ ) {
        step = (Step*) ui->stepTab->widget( i);
        if ( i != step->index() ) {
            oldix.append( step->index());
            newix.append( i);
            step->setIndex( i);
            QStringList sl = ui->stepTab->tabText( i).split( ' ');
            if ( sl.first().isEmpty() || (sl.first() == steptxt && sl.last().toInt() > 0) )
                // only renumber a tab if it hasn't been renamed
                ui->stepTab->setTabText( i, steptxt + " " + QString::number( i));
        }
    }

    // renumber all stepper tiles
    for ( int s = 1; s < ui->stepTab->count() - 1; s++ ) {
        step = (Step*) ui->stepTab->widget( s);
        Tile* tile = (Tile*) step->firstTile();
        while ( tile ) {
            Template *temp = tile->templ();
            QString key = temp->alias() + "_GOTOSTEP";
            Var* var = temp->m_vars.firstVar( vtSignalIn);
            while ( var ) {
                if ( var->key == key ) {
                    Field* fld = tile->field( var->alias);
                    for ( int i = 0; i < oldix.count(); i++ )
                        if ( fld->value().toInt() == oldix.at( i) ) {
                            fld->setValue( QString::number( newix.at( i)), true);
                            break;
                        }
                }
                var = temp->m_vars.nextVar( vtSignalIn);
            }
            tile = step->nextTile();
        }
    }
}

Tile* MainWindow::createDevice( QString filepath, QString device, QString alias)
// filepath : full path including the filename of the pid-file
// device / alias : supply a device name and its alias in case of opening a saved program
{
    QString path = filepath.left( filepath.lastIndexOf( '/') + 1);
    QString filename = filepath.right( filepath.length() - filepath.lastIndexOf( '/') - 1);

    QFile file( filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return NULL;
    }

    Step* step = (Step*) ui->stepTab->widget( 0);
    Template* tmpl = NULL;
    Tile* tile = NULL;
    QTextStream in(&file);
    QString line, ln;

    // check the intended platform
    while ( !in.atEnd() ) {
        line = in.readLine().trimmed();
        if ( line.left( 2) == "//" ) continue;
        if ( line.isEmpty() ) continue;
        if ( line.toUpper() == "<PLATFORM>" ) {
            while ( !in.atEnd() ) {
                line = in.readLine().trimmed();
                if ( line.isEmpty() ) continue;
                if ( line.toUpper() != m_platform.toUpper() ) {
                    message( QMessageBox::Information, tr( "Dit apparaat kan niet worden gebruikt.\nHet hoort bij een ander platform."));
                    goto skiptemplates;
                }
                goto starttemplates;
            }
        }
    }

starttemplates:

    // get the templates
    while ( !in.atEnd() ) {
        ln = in.readLine();
        line = ln.trimmed();
        if ( line.isEmpty() ) continue;

        if ( line.toUpper() == "<DEVICE>" ) {
            // start new template
            tmpl = new Template( this);
            tmpl->setAlias( alias);
            tmpl->setPath( path);
            tmpl->setFilename( filename);
            lockPlatform();
        }
        else
        if ( line.toUpper() == "<END>" ) {
            if ( tmpl ) {
                // check if it was the requested device
                // (in case of opening a saved program)
                if ( !device.isEmpty() && (device != tmpl->device() ) ) {
                    delete tmpl;
                    tmpl = NULL;
                    continue;
                }

                QString msg;

                // if no alias came up from the template file, ask the user for one
                if ( tmpl->alias().isEmpty() ) {
askalias:
                    TileId dlgId( tmpl->device());
                    if ( dlgId.exec() == QDialog::Accepted ) {
                        tmpl->setAlias( dlgId.getTileId());

                        // check if the alias is filled in
                        // only occurs when the user doesn't supply an alias
                        if ( tmpl->alias().isEmpty() ) {
                            msg = tr( "Apparaat '") + tmpl->device() + tr( "' heeft geen naam\nen kan niet worden toegevoegd.");
                            message( QMessageBox::Warning, msg);
                            delete tmpl;
                            continue;
                        }
                    }
                    else {
                        delete tmpl;
                        continue;
                    }
                }

                // check if the alias has not been used yet
                if ( (tile = step->tile( tmpl->alias())) ) {
                    msg = tr( "Apparaat '") + tmpl->alias() + tr( "' bestaat al.\nBedenk een andere naam.");
                    message( QMessageBox::Warning, msg);
                    goto askalias;
                }

                // alias should start with a character
                if ( !QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ").contains( tmpl->alias().left( 1), Qt::CaseInsensitive) ) {
                    msg = tr( "Apparaatnaam hoort met een letter te beginnen.");
                    message( QMessageBox::Warning, msg);
                    goto askalias;
                }

                // alias may not contain special charaters
                QString special = "[]{}().,-+*/\\<>|&^%$#!~\"'`:;?\t\n\r";
                for ( int i = 0; i < special.length(); i++ )
                    if ( tmpl->alias().contains( special.at( i)) ) {
                        msg = tr( "Apparaatnaam mag geen speciale lettertekens bevatten.");
                        message( QMessageBox::Warning, msg);
                        goto askalias;
                    }

                // when everything ok:
                tmpl->replaceKeys();                    // make keys alias specific
                tile = tmpl->createDevice();            // create the tile
                insertDevice( tmpl, m_templates.count());
                g_history.deviceCreated( tmpl, m_templates.count());

                tmpl = NULL;
            }
        }
        else
            if ( tmpl ) tmpl->parseLine( ln, m_platform, filename.left( filename.lastIndexOf( '.')));
    }

skiptemplates:

    file.close();

    return tile;
}

Tile* MainWindow::createTile( int stepix, QString alias)
{
    Step* step;
    Template* tmpl = NULL;
    Tile* tile = NULL;
    int i;

    step = (Step*) ui->stepTab->widget( stepix);

    for ( i = 0; i < m_templates.count(); i++ )
        if ( (tmpl = m_templates.at( i))->alias() == alias ) break;
    tile = step->firstTile();
    while ( tile ) {
        if ( tile->alias() == alias ) {
            message( QMessageBox::Information, tr( "Dit apparaat is al toegevoegd aan deze stap."));
            return NULL;
        }
        tile = step->nextTile();
    }
    if ( i < m_templates.count() )
        tile = tmpl->createTile();

    adjustScroll();
    return tile;
}

void MainWindow::insertDevice( Template* templ, int index)
{
    m_templates.insert( index, templ);
    ((Step*) ui->stepTab->widget( 0))->appendTile( templ->deviceTile());
    createTool( templ);
}

void MainWindow::removeDevice( QString alias)
{
    int i;
    for ( i = 0; i < m_templates.count(); i++ )
        if ( m_templates.at( i)->alias() == alias ) break;
    if ( i >= m_templates.count() ) return;
    Template* templ = m_templates.at( i);
    deleteToolSignals( templ);
    ((Step*) ui->stepTab->widget( 0))->removeTile( alias);
    m_templates.removeAt( i);
}

void MainWindow::unselectSignal()
{
    QModelIndex ix = m_smodel.index( -1);
    ui->listSignals->setCurrentIndex( ix);
}

void MainWindow::on_butRename_clicked()
{
    if ( !ui->stepTab->currentIndex() ) return;

    RenameStep rs;

    if ( rs.exec() == QDialog::Accepted ) {
        int ix = ui->stepTab->currentIndex();
        g_history.stepRenamed( ui->stepTab->currentIndex(), ui->stepTab->tabText( ix), rs.name());
        ui->stepTab->setTabText( ix, rs.name());
    }
}

void MainWindow::on_butDelete_clicked()
{
    int ix = ui->stepTab->currentIndex();

    // don't delete the 'config' or the 'new step' tap
    if ( !ix || (ix == ui->stepTab->count() - 1) ) return;

    // warn if the step contains tiles
    Step* step = (Step*) ui->stepTab->currentWidget();
    if ( step->children().count() &&
         (message( QMessageBox::Question, tr( "Deze stap bevat nog tegels."), tr( "Toch deze stap verwijderen?")) != QMessageBox::Yes) )
        return;

    // actually remove the tab
    g_history.stepDeleted( step, ix, ui->stepTab->tabText( ix));
    removeStep( ix);
}

void MainWindow::on_adddevice_triggered()
{
    DeviceViewer dv( this);
    if ( dv.exec() == QDialog::Rejected )
        return;
    QStringList devices = dv.devices();
    for ( int i = 0; i < devices.count(); i++ )
        createDevice( devices.at( i));
    return;
}

void MainWindow::on_actuatordevice_triggered()
{
    if ( !ui->stepTab->currentIndex() ) {
        message( QMessageBox::Information, tr( "Het is niet mogelijk om tegels op\nhet blad 'Instellingen' te plaatsen."));
        return;
    }
    QString alias = ((QAction*) sender())->iconText();
    Tile* tile = createTile( ui->stepTab->currentIndex(), alias);
    if ( tile ) {
        g_history.tileCreated( tile, tile->step()->ixTile( tile->templ()->alias()));
        setConstants( tile->templ());
    }
}

void MainWindow::on_signaldevice_triggered()
{
    QString alias = ((QAction*) sender())->iconText();
    Template* tmpl = ((Step*)ui->stepTab->widget( 0))->tile( alias)->templ();
    setSignals( tmpl);
    setConstants( tmpl);
}

void MainWindow::on_stepTab_currentChanged(int index)
{
    ui->listSignals->setEnabled( ui->stepTab->currentIndex() ? true : false);
    ui->listConstants->setEnabled( ui->stepTab->currentIndex() ? true : false);

    if ( index == ui->stepTab->count() - 1 ) {
        if ( !index ) {
            ui->stepTab->insertTab( 0, new Step( this), tr( "Instellingen"));
            ui->stepTab->setStyleSheet( "background-image:url(:/background.png); color:blue");
        }
        else {
            if ( !m_movestep ) {
                if ( m_curstep < 0 || m_curstep >= ui->stepTab->count() - 2 )
                    m_curstep = index;
                else
                    m_curstep++;
                Step *step = new Step( this);
                insertStep( step, m_curstep);
                g_history.stepCreated( step, m_curstep, ui->stepTab->tabText( m_curstep));
            }
        }
    }

    Step *step = (Step*) ui->stepTab->currentWidget();
    ui->scrollTab->setRange( 0, step->sizeY() > ui->scrollTab->height() ? step->sizeY() - ui->scrollTab->height() : 0);
    ui->scrollTab->setSliderPosition( -step->getScroll());
    m_curstep = ui->stepTab->currentIndex();
}

void MainWindow::on_scrollTab_valueChanged(int value)
{
    Step *step = (Step*) ui->stepTab->currentWidget();
    step->setScroll( -value);
    Tile* tile = step->firstTile();
    while ( tile ) {
        tile->scrollTo( -value);
        tile = step->nextTile();
    }
}

bool MainWindow::askSave()
{
    QString mydocs = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation);
    QFileDialog fd( this, tr( "Bewaar dit project ..."), mydocs, "ProtoIt Projects (*.pip)");
    fd.setOption( QFileDialog::DontUseNativeDialog);
    fd.setAcceptMode( QFileDialog::AcceptSave);
    if ( fd.exec() == QDialog::Rejected )
        return false;

    QString oldproject = m_project;
    m_project = fd.selectedFiles().first();
    if ( m_project.lastIndexOf( '.') <= m_project.lastIndexOf( '/') )
        m_project += ".pip";

    QString dest = projectPath();
    QDir dir( dest);
    if ( dir.exists() ) {
        if ( message( QMessageBox::Question, tr( "De apparaten voor dit project worden bewaard in:\n")
                      + dest + tr( "\n\nDeze map bestaat al."), tr( "De map toch gebruiken?")) == QMessageBox::No ) {
            m_project = oldproject;
            return false;
        }
    }
    else
        dir.mkpath( dest);

    return true;
}

bool MainWindow::checkSave()
{
    if ( !g_history.isSaved() ) {
        int ret = message( QMessageBox::Question, tr( "De veranderingen zijn nog niet bewaard."), tr( "Nu eerst opslaan?"));
        if ( ret == QMessageBox::Cancel )
            return false;
        if ( ret == QMessageBox::Yes ) {
            if ( !askSave() )
                return false;
            doSave();
        }
    }
    return true;
}

void MainWindow::saveDevice( QString srcpath, QString dstpath, QString device)
{
    // 'srcpath' and 'dstpath' need to point to the folder containing the subfolders 'Models', 'Devices' and 'Functions'
    // 'device' must have one of these subfolders followed by the pid-filename.

    int ix;
    if ( (ix = device.lastIndexOf( '/')) >= 0 ) { // should always be so
        srcpath += device.left( ix + 1);
        dstpath += device.left( ix + 1);
        device = device.right( device.length() - ix - 1);
    }

    QDir dir;
    QStringList files;
    QString subdir;

    // copy the pid-file of the device
    subdir = device.left( device.lastIndexOf( '.')) + "/";
    dir.mkpath( dstpath + subdir);
    QFile::copy( srcpath + device, dstpath + device);

    // copy the supporting files of the device
    srcpath += subdir;
    dstpath += subdir;
    dir.setPath( srcpath);
    files = dir.entryList( QDir::Files);
    for ( int i = 0; i < files.count(); i++ )
        QFile::copy( srcpath + files.at( i), dstpath + files.at( i));
}

void MainWindow::doSave()
{
    QFile file( m_project);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        message( QMessageBox::Warning, tr( "Kon de besturing niet bewaren.") );
        return;
    }
    QTextStream out( &file);

    Step* step = (Step*) ui->stepTab->widget( 0);
    Tile* tile;
    Field* field;
    QString stepalias;
    QString source;
    QString path;
    QStringList sources;
    QString sep = "/#>";
    int i, ix;

    out << "PLATFORM" << sep << m_platform << "\n";

    for ( i = 0; i < ui->stepTab->count() - 1; i++ ) {

        step = (Step*) ui->stepTab->widget( i);
        if ( i ) {
            stepalias = ui->stepTab->tabText( i);
            if ( stepalias.left( 5) == tr( "Stap ") )
                out << "\nSTEP" << sep << QString::number( i) << "\n";
            else
                out << "\nSTEP" << sep << QString::number( i) << sep << stepalias << "\n";
        }

        tile = step->firstTile();
        while ( tile ) {
            out << "\n";

            if ( !i ) {
                path = tile->templ()->path();
                path = path.left( path.length() - 1);
                if ( (ix = path.lastIndexOf( '/')) >= 0 ) // should always be so
                    path = path.right( path.length() - ix - 1) + '/';
                out << "DEVICE" << sep << path + tile->templ()->filename() << sep << tile->templ()->device() << sep << tile->alias() << "\n";
                source = tile->templ()->path() + tile->templ()->filename();
                if ( !sources.contains( source) ) {
                    sources.append( source);
                    saveDevice( tile->templ()->path(), projectPath() + path, tile->templ()->filename());
                }
            }
            else
                out << "TILE" << sep << tile->alias() << "\n";

            field = tile->firstField();
            while ( field ) {
                out << "FIELD" << sep << field->alias() << sep << field->value() << "\n";
                field = tile->nextField();
            }

            tile = step->nextTile();
        }
    }

    file.close();


    g_history.setSaved();
    setTitle();
}

void MainWindow::doNew()
{
    g_history.removeAll();
    g_history.setSaved();

    m_project = NEWPROJECT;

    for ( int i = 0; i < m_templates.count(); i++ ) {
        deleteToolSignals( m_templates.at( i));
        m_signals.clear();
        m_constants.clear();
        m_smodel.setStringList( m_signals);
        m_cmodel.setStringList( m_constants);
    }

    Step* step;
    ui->stepTab->setCurrentIndex( 0);

    step = (Step*) ui->stepTab->widget( 0);
    step->deleteAllTiles();

    while ( ui->stepTab->count() > 2 ) {
        step = (Step*) ui->stepTab->widget( 1);
        ui->stepTab->removeTab( 1);
        delete step;
    }

    for ( int i = 0; i < m_templates.count(); i++ )
        delete m_templates.at( i);
    m_templates.clear();

    step = (Step*) ui->stepTab->currentWidget();
    ui->scrollTab->setRange( 0, step->sizeY() > ui->scrollTab->height() ? step->sizeY() - ui->scrollTab->height() : 0);
    ui->scrollTab->setSliderPosition( -step->getScroll());

    freePlatform();
    setTitle();
}

void MainWindow::on_actionNew_triggered()
{
    if ( !checkSave() ) return;
    doNew();
}

void MainWindow::doOpen( QString project)
{
    QFile file( project);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    bool success = true;
    m_silent = true;
    m_project = project;

    QTextStream in( &file);

    Step* step = (Step*) ui->stepTab->widget( 0);
    Tile* tile = NULL;
    QStringList line;
    QString ln;
    int stepix = 0;
    int fieldix = 0;
    QString path;
    QString filename;
    QString tilealias;
    QString msg;

    while ( !in.atEnd() ) {
        ln = in.readLine().trimmed();
        if ( ln.isEmpty() ) continue;

        line = ln.split( "/#>");
        if ( line.count() < 2 ) {
            success = false;
            continue;
        }

        if ( line.first() == "PLATFORM") {
            m_platform = line.last().toUpper();
            int i;
            for ( i = 0; i < m_versions.count(); i++ )
                if ( m_versions.at( i)->platform.toUpper() == m_platform ) break;
            if ( i >= m_versions.count() ) {
                file.close();
                m_silent = false;
                message( QMessageBox::Warning, tr( "Het platform van dit project wordt in deze installatie van ProtoIt niet ondersteund.\nHet project kan niet worden geopend."));
                return;
            }
            m_curversion = i;
            m_version = m_versions.at( i);
            m_platform = m_version->platform;
            for ( int i = 0; i < m_versions.count(); i++ )
                m_versions.at( i)->action->setChecked( m_curversion == i);
            lockPlatform();
            setMenu();
        }

        if ( line.first() == "DEVICE" ) {
            if ( line.count() != 4 ) {
                success = false;
                msg += tr( "- Er ontbreekt een apparaat.");
                continue;
            }
            filename = line.at( 1);
            // first try in project path
            path = projectPath();
            QDir dir;
            dir.setPath( path);
            // if no longer exists, try in platform path
            if ( !dir.exists() ) {
                path = platformPath() + "Devices/";
                dir.setPath( path);
                if ( !dir.exists() ) {
                    success = false;
                    msg += tr( "\n- Apparaat '") + line.at( 2) + tr( "' werd niet gevonden.");
                }
            }
            tile = createDevice( path + filename, line.at( 2), line.at( 3));
            fieldix = 0;
            // g_history record is created in createDevice
        }

        if ( line.first() == "STEP" ) {
            stepix = line.at( 1).toInt();
            if ( stepix ) {
                step = new Step( this);
                step->setIndex( stepix);
                if ( line.count() > 2 )
                    ui->stepTab->insertTab( ui->stepTab->count() - 1, step, line.last());
                else
                    ui->stepTab->insertTab( ui->stepTab->count() - 1, step, tr( "Stap ") + line.last());
                ui->stepTab->setCurrentIndex( stepix);
            }
            else {
                success = false;
                msg += tr( "\n- Poging om het tabblad 'Instellingen' te overschijven.");
            }
        }

        if ( line.first() == "TILE" ) {
            if ( !line.last().isEmpty() ) {
                tilealias = line.last();
                tile = createTile( stepix, tilealias);
                fieldix = 0;
            }
            if ( line.last().isEmpty() || !tile ) {
                success = false;
                msg += tr( "- Er ontbreekt een programmategel.");
                continue;
            }
        }

        if ( line.first() == "FIELD" ) {
            if ( tile ) {
                if ( (line.count() != 3) || line.at( 1).isEmpty() ) {
                    success = false;
                    msg += tr( "- Er ontbreekt een veld.");
                    continue;
                }
                tile->initField( line.at( 1), line.last(), fieldix);
                fieldix++;
            }
        }
    }

    file.close();
    m_silent = false;

    if ( !success )
        message( QMessageBox::Critical, tr( "Het programma bevat de volgende fouten:\n\n") + msg);

    g_history.removeAll();
    g_history.setSaved();

    lockPlatform();
    setTitle();
}

void MainWindow::on_actionOpen_triggered()
{
    if ( !checkSave() ) return;
    doNew();

    QString mydocs = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation);
    QFileDialog fd( this, tr( "Doorgaan met een project ..."), mydocs, "ProtoIt Projects (*.pip)");
    fd.setOption( QFileDialog::DontUseNativeDialog);
    fd.setAcceptMode( QFileDialog::AcceptOpen);
    if ( fd.exec() == QDialog::Rejected )
        return;

    doOpen( fd.selectedFiles().first());
}

void MainWindow::on_actionSave_triggered()
{
    if ( (m_project == NEWPROJECT) && !askSave() )
        return;
    doSave();
}

void MainWindow::on_actionSaveAs_triggered()
{
    if ( askSave() ) doSave();
}

void MainWindow::on_actionUpload_triggered()
{
    setCursor( Qt::WaitCursor);
    QString curdir = QDir::currentPath();

    if ( m_serial.isOpen() ) serialClose();

    writeCode();
    uploadCode();

    QDir::setCurrent( curdir);
    setCursor( Qt::ArrowCursor);
}

QString MainWindow::replaceParam( QString cmd, QString param, QStringList& items)
{
    QString str, lstr, mstr, rstr;
    int i, i1, i2 = 0;
    QString prm = "%" + param + "%";
    while ( (i1 = cmd.indexOf( "[[", i2) + 2) >= i2 + 2 ) {
        if ( (i2 = cmd.indexOf( "]]", i1)) > i1 ) {
            mstr = cmd.mid( i1, i2 - i1);
            if ( mstr.indexOf( prm) >= 0 ) {
                lstr = cmd.left( i1 - 2);
                rstr = cmd.right( cmd.length() - i2 - 2);
                for ( i = 0; i < items.count(); i++ ) {
                    str = mstr;
                    str = str.replace( prm, items.at(i));
                    lstr += str;
                }
                cmd = lstr + rstr;
            }
        }
    }
    return cmd;
}

QString MainWindow::replaceParam( QString cmd, QString param, QString item)
{
    QString prm = "%" + param + "%";
    while ( cmd.indexOf( prm) >= 0 ) cmd = cmd.replace( prm, item);
    return cmd;
}

void MainWindow::killProtoItRun()
{
    QProcess proc;
    proc.start( "pidof ProtoItRun");
    proc.waitForFinished();
    QString pid = proc.readAllStandardOutput();
    if ( !pid.isEmpty() )
        QProcess::execute( "kill " + pid);
}

void MainWindow::uploadCode()
{
    int i, j;
    QString line, subm;
    QString buildpath = buildPath();
    QStringList cmmds;
    QString curdir = QDir::currentPath();
    QDir::setCurrent( programmerPath());
    QStringList libs;

#ifdef Q_OS_WIN
    buildpath = toMsDos( buildpath); // needed for avr-g++ compiler
#endif

    if ( !m_version ) return;
    if ( m_version->programmer.toUpper() == "WINAVR" )
        m_serialport = findSerialPort();
    if ( m_version->programmer.toUpper() == "NIXG")
        killProtoItRun();

    // amass all libraries needed for linking
    for ( i = 0; i < m_templates.count(); i++ ) {
        Template* templ = m_templates.at( i);
        for ( j = 0; j < templ->m_libs.count(); j++ )
            if ( !libs.contains( templ->m_libs.at( j), Qt::CaseInsensitive) )
                libs.append( templ->m_libs.at( j));
    }

    // COMPILE MAIN
    if ( m_version->compilemain.count() ) {
        line = m_version->compilemain.at( 0);
        line = replaceParam( line, "buildpath", buildpath);
        line = replaceParam( line, "library", libs);
        cmmds.append( line);
#ifdef Q_OS_WIN
        line.replace( '/', '\\'); // needed for nbc programmer
#endif
    }

    // COMPILE LIBRARIES
    if ( m_version->compilelibs.count() ) {
        for ( i = 0; i < libs.count(); i++ ) {
            line = m_version->compilelibs.at( 0);
            line = replaceParam( line, "buildpath", buildpath);
            line = replaceParam( line, "library", libs.at( i));
            cmmds.append( line);
#ifdef Q_OS_WIN
        line.replace( '/', '\\'); // needed for nbc programmer
#endif
        }
    }

    // BUILD
    if ( m_version->build.count() ) {
        line = m_version->build.at( 0);
        line = replaceParam( line, "buildpath", buildpath);
        line = replaceParam( line, "library", libs);
        cmmds.append( line);
#ifdef Q_OS_WIN
        line.replace( '/', '\\'); // needed for nbc programmer
#endif
    }

    // LINK
    for ( i = 0; i < m_version->link.count(); i++ ) {
        line = m_version->link.at( i);
        line = replaceParam( line, "buildpath", buildpath);
        line = replaceParam( line, "library", libs);
#ifdef Q_OS_WIN
        line.replace( '/', '\\'); // needed for nbc programmer
#endif
        cmmds.append( line);
    }

    // PREPARE MESSAGE

    QString msg;
    if ( m_version->message.count() )
        msg = m_version->message.at( 0);

    // UPLOAD
    if ( m_version->upload.count() ) {
        line = m_version->upload.at( 0);
        line = replaceParam( line, "buildpath", buildpath);
        line = replaceParam( line, "port", m_serialport);
#ifdef Q_OS_WIN
        line.replace( '/', '\\'); // needed for nbc programmer
#endif
        cmmds.append( line);
    }

    for ( int i = 0; i < cmmds.count(); i++ ) {
        line = cmmds.at( i);
        if ( (j = line.indexOf( ' ')) >= 0 ) {
            subm = line.left( j).toUpper();
            line = line.right( line.length() - j - 1).trimmed();
        }
        else
            subm = "EXEC";

        int ret;
        QProcess proc;

        if ( subm == "SUBMIT" ) {
            proc.start( line);
            proc.waitForFinished();
            ret = (proc.exitStatus() == QProcess::NormalExit ? proc.exitCode() : -1);
        }
        else
        if ( subm == "EXEC" ) {
            proc.start( line);
            proc.waitForFinished();
            ret = (proc.exitStatus() == QProcess::NormalExit ? proc.exitCode() : -1);
        }
        else
        if ( subm == "START" ) {
            ret = !proc.startDetached( line);
        }

        // MESSAGE

        if ( ret ) {
            QString fail = tr( "Upload mislukte door de volgende fout in:\n\n") + line + "\n\n";
            QString err = proc.readAllStandardError();
            if ( i < cmmds.count() - 1 )
                message( QMessageBox::Critical, fail + err + tr( "\n\n") + msg);
            else {
                if ( (m_version->programmer.toUpper() != "NBC") || err.isEmpty() ) {
                    msg += line + "\n\n";
                    message( QMessageBox::Critical, tr( "Upload is mislukt\n\nIs de robot aangesloten?\n\n") + msg);
                }
                else
                    message( QMessageBox::Critical, fail + err + tr( "\n\n") + msg);
            }
            goto restorepath;
        }
    }

    message( QMessageBox::Information, tr( "Upload is gelukt\n\n") + msg);

restorepath:
    QDir::setCurrent( curdir);
    setCursor( Qt::ArrowCursor);

}

void MainWindow::writeCode()
{
    int i, j;
    QString line, s;
    QStringList copyrights;
    QStringList includes;
    bool stepper;
    QString stepperstr = ":/stepper.png";

    if ( !m_version ) return;

    QFile file( buildPath() + m_version->outputfile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);

    // HEADER
    for ( i = 0; i < m_version->header.count(); i++ )
        out << m_version->header.at( i) + "\n";
    out << "\n";

    // COPYRIGHTS
    for ( i = 0; i < m_templates.count(); i++ ) {
        m_templates.at( i)->m_code->tabs( 0);
        out << m_templates.at( i)->m_code->writeCopyright( m_templates.at( i)->device(), copyrights);
        for ( j = 0; j < m_templates.at( i)->m_code->m_copyright.count(); j++ ) {
            s = m_templates.at( i)->m_code->m_copyright.at( j);
            if ( !copyrights.contains( s) )
                copyrights.append( s);
        }
    }
    out << "\n";

    // LICENSE
    for ( i = 0; i < m_version->license.count(); i++ )
        out << m_version->license.at( i) + "\n";
    out << "\n";

    // DEFINES
    for ( i = 0; i < m_version->define.count(); i++ )
        out << m_version->define.at( i) + "\n";
    out << "\n";

    // INCLUDE FILES
    for ( i = 0; i < m_version->include.count(); i++ )
        out << m_version->include.at( i) + "\n";
    for ( i = 0; i < m_templates.count(); i++ ) {
        m_templates.at( i)->m_code->tabs( 0);
        out << m_templates.at( i)->m_code->writeInclude( includes);
        for ( j = 0; j < m_templates.at( i)->m_code->m_include.count(); j++ ) {
            s = m_templates.at( i)->m_code->m_include.at( j);
            if ( !includes.contains( s) )
                includes.append( s);
        }
    }
    out << "\n";

    // GLOBAL STEP INDEX
    out << "int LASTSTEP = " << ui->stepTab->count() - 2 << ";\n";
    out << "int STEP = 1;\n\n";

    // GLOBAL VARIANT DECLARATIONS
    QStringList vars;
    QStringList vals;
    QStringList vardecl;
    Step* step = (Step*)ui->stepTab->widget( 0);
    for ( i = 0; i < m_templates.count(); i++ ) {
        Template* tmpl = m_templates.at( i);
        if ( tmpl->deviceTile()->isHidden() ) continue;
        Var* var = tmpl->m_vars.firstVar( vtAll);
        while ( var ) {
            if ( var->vt == vtGlobal ) {
                var = tmpl->m_vars.nextVar( vtAll);
                continue; // vtGlobal and vtLocal are handled by the codewriter
            }
            line = "Variant " + var->key;
            if ( var->vt == vtPrivate ) {
                if( !var->init.isEmpty() ) {
                    vars.append( var->key);
                    vals.append( var->init);
                }
            }
            else {
                Field* field = step->tile( tmpl->alias())->field( var->alias);
                // SIGNALIN and CONSTANT don't have a field for the device tile
                if ( field && !field->value().isEmpty() ) {
                    vars.append( var->key);
                    vals.append( field->value());
                }
            }
            line += ";\n";
            if ( !vardecl.contains( line)) {
                vardecl.append( line);
                out << line;
            }
            var = tmpl->m_vars.nextVar( vtAll);
        }
    }
    out << "\n";

    // GLOBAL DECLARATIONS
    for ( i = 0; i < m_version->declaration.count(); i++ )
        out << m_version->declaration.at( i) + "\n";
    out << "\n";
    for ( i = 0; i < m_templates.count(); i++ ) {
        m_templates.at( i)->m_code->tabs( 0);
        out << m_templates.at( i)->m_code->writeGlobal();
    }
    out << "\n";

    // ROUTINES
    for ( i = 0; i < m_version->routine.count(); i++ )
        out << m_version->routine.at( i) + "\n";
    out << "\n";
    QStringList devices;
    Template* tmpl;
    for ( i = 0; i < m_templates.count(); i++ ) {
        tmpl = m_templates.at( i);
        if ( devices.contains( tmpl->device()) ) continue;
        devices.append( tmpl->device());
        tmpl->m_code->tabs( 0);
        out << tmpl->m_code->writeRoutine();
    }
    out << "\n";

    // DATAFEEDTHROUGH
    out << "void dataFeedThrough() {\n";
    out << "\n\tswitch ( STEP ) {\n";
    for ( i = 1; i < ui->stepTab->count() - 1; i++ ) {
        step = (Step*)ui->stepTab->widget( i);
        out << "\n\tcase " << QString::number( i) << " :\n";

        Tile* tile = step->firstTile();
        while ( tile ) {
            tile->templ()->m_code->tabs( 2);
            out << tile->templ()->m_code->writeDataFeedthrough( m_version->initvariant, tile);
            tile = step->nextTile();
        }
        out << "\t\tbreak;\n";
    }
    out << "\t}\n}\n\n";

    // STEPINIT
    out << "void stepInit()\n{\n";
    out << "\tdataFeedThrough();\n\n";
    for ( i = 0; i < m_templates.count(); i++ ) {
        m_templates.at( i)->m_code->tabs( 0);
        out << m_templates.at( i)->m_code->writeStepInit();
    }
    out << "}\n\n";

    // SETUP
    out << "void setup()\n{\n";
    for ( i = 0; i < m_version->setup.count(); i++ )
        out << m_version->setup.at( i) + "\n";
    out << "\n";
    for ( i = 0; i < vars.count(); i++ ) { // init signal declarations
        if ( !vals.at( i).isEmpty() ) {
            line = m_version->initvariant;
            line = replaceParam( line, "variant", vars.at( i));
            line = replaceParam( line, "value", vals.at( i));
            out << "\t" << line << "\n";
        }
    }
    out << "\n";
    for ( i = 0; i < m_templates.count(); i++ ) {
        m_templates.at( i)->m_code->tabs( 0);
        out << m_templates.at( i)->m_code->writeSetup();
    }
    out << "\n\tstepInit();\n}\n\n";

    // LOOP
    out << "void loop() {\n";
    for ( i = 0; i < m_version->loop.count(); i++ )
        out << m_version->loop.at( i) + "\n";
    out << "\n";

    out << "\t// SENSOR UPDATE\n";
    for ( i = 0; i < m_templates.count(); i++ ) {
        m_templates.at( i)->m_code->tabs( 0);
        out << m_templates.at( i)->m_code->writeSensorUpdate();
    }
    out << "\n\t// DATA FEED THROUGH\n";
    out << "\tdataFeedThrough();\n";

    out << "\n\tswitch ( STEP ) {\n";
    stepper = false;
    for ( i = 1; i < ui->stepTab->count() - 1; i++ ) {
        step = (Step*)ui->stepTab->widget( i);
        Tile* tile = step->firstTile();
        out << "\n\tcase " << QString::number( i) << " :\n";

        out << "\n\t\t// FUNCTION UPDATE\n";
        tile = step->firstTile();
        while ( tile ) {
            if ( tile->templ()->icon().toLower() != stepperstr ) {
                tile->templ()->m_code->tabs( 1);
                out << tile->templ()->m_code->writeFunctionUpdate();
            }
            else
                stepper = true;
            tile = step->nextTile();
        }
        out << "\n\t\t// DATA FEED THROUGH\n";
        out << "\t\tdataFeedThrough();\n";

        out << "\n\t\t// ACTUATOR UPDATE\n";
        tile = step->firstTile();
        while ( tile ) {
            tile->templ()->m_code->tabs( 1);
            out << tile->templ()->m_code->writeActuatorUpdate();
            tile = step->nextTile();
        }
        if ( stepper ) {
            out << "\n\t\t// STEP TO\n";
            tile = step->firstTile();
            while ( tile ) {
                if ( tile->templ()->icon().toLower() == stepperstr ) {
                    tile->templ()->m_code->tabs( 1);
                    out << tile->templ()->m_code->writeFunctionUpdate();
                }
                tile = step->nextTile();
            }
        }
        out << "\t\tbreak;\n";
    }
    out << "\t}\n}\n\n";

    // MAIN
    for ( i = 0; i < m_version->maintask.count(); i++ )
        out << m_version->maintask.at( i) + "\n";
    out << "\n";

    file.close();
}

void MainWindow::on_actionExport_triggered()
{
    if ( !m_version ) {
        message( QMessageBox::Information, tr( "Er is een fout opgetreden wat betreft het platform.\nExporteren is niet mogelijk."));
        return;
    }

    QString mydocs = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation);
    QString fname, fext = "*";
    int ix;

    if ( (ix = m_version->outputfile.lastIndexOf( '.')) >= 0 )
        fext += m_version->outputfile.right( m_version->outputfile.length() - ix);

    QFileDialog fd( this, tr( "Exporteer code"), mydocs, "ProtoIt code (" + fext + ")");
    fd.setOption( QFileDialog::DontUseNativeDialog);
    fd.setAcceptMode( QFileDialog::AcceptSave);
    if ( fd.exec() == QDialog::Rejected )
        return;
    fname = fd.selectedFiles().first();
    if ( fname.lastIndexOf( '.') <= fname.lastIndexOf( '/') )
        fname += fext.right( fext.length() -1);

    writeCode();
    QFile::copy( buildPath() + m_version->outputfile, fname);
    QString msg = tr( "De code is opgeslagen. Let op: deze code is platformafhankelijk. ");
    msg += tr( "Header-files, zoals die waar de 'Variant' in wordt gedeclareerd, zijn te vinden in de 'programmer'-map van de ProtoIt installatie.");
    message( QMessageBox::Information, msg);
}

void MainWindow::on_actionPlatform_triggered()
{
    int ix;
    for ( ix = 0; ix < m_versions.count(); ix++ ) {
        if ( (ix != m_curversion) && m_versions.at( ix)->action->isChecked() ) {
            m_platform = m_versions.at( ix)->platform;
            m_version = m_versions.at( ix);
            break;
        }
    }
    if ( m_curversion >= 0 && m_curversion < m_versions.count() )
        m_versions.at( m_curversion)->action->setChecked( false);
    m_curversion = ix;

    setMenu();
}

void MainWindow::on_actionQuit_triggered()
{
    if ( !checkSave() ) return;
    qApp->quit();
}

void MainWindow::on_actionUndo_triggered()
{
    g_history.undo();
}

void MainWindow::on_actionRedo_triggered()
{
    g_history.redo();
}

void MainWindow::on_actionReport_triggered()
{
    g_history.report();
}

void MainWindow::on_actionHelp_triggered()
{
    HelpViewer hv( this);
    hv.setHelp( homePath() + "help/ProtoIt.pih");

    QString tmpl;
    QStringList tmplist;
    QStringList pathlist;
    Step* step = (Step*) ui->stepTab->widget( 0);
    Tile* tile = step->firstTile();
    while ( tile ) {
        tmpl = tile->templ()->filename();
        tmpl = tmpl.left( tmpl.lastIndexOf( '.'));
        if ( !tmplist.contains( tmpl) ) {
            tmplist.append( tmpl);
            pathlist.append( tile->templ()->path());
        }
        tile = step->nextTile();
    }

    hv.exec();
}

void MainWindow::on_actionAbout_triggered()
{
    Info info( this);
    info.exec();
}

void MainWindow::on_actionDatalogger_triggered()
{
    QStringList dl;
    QString fname;

    QFileDialog fd( this, tr( "Bestand voor de datalogger-gegevens"), "ProtoIt.csv ", "Excel (*.csv)");
    fd.setOption( QFileDialog::DontUseNativeDialog);
    fd.setAcceptMode( QFileDialog::AcceptSave);
    if ( fd.exec() == QDialog::Rejected )
        return;
    fname = fd.selectedFiles().first();
    if ( fname.lastIndexOf( '.') <= fname.lastIndexOf( '/') )
        fname += ".csv";

    QFile file( fname);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        message( QMessageBox::Warning, tr( "Bestand werd niet geopend."));
        return;
    }
    QTextStream out( &file);

    setCursor( Qt::WaitCursor);

    if ( !serialOpen() ) {
        message( QMessageBox::Warning, tr( "De seriële communicatie mislukte.\nEr is niets opgeslagen."));
        goto datalogfailure;
    }

    // command for retrieving datalogger data
    if ( !serialWrite( "RTCMDDATA") ) {
        message( QMessageBox::Warning, tr( "De datalogger werd niet gevonden.\nEr is niets opgeslagen."));
        goto datalogfailure;
    }
    dl = serialRead();

    for ( int i = 0; i < dl.count(); i++ )
        out << dl.at( i);

    message( QMessageBox::Information, tr( "Het datalogger bestand is opgeslagen."));

datalogfailure:
    file.close();
    setCursor( Qt::ArrowCursor);
}

void MainWindow::on_actionStop_triggered()
{
    if ( !m_version || m_version->stoprobot.isEmpty() ) return;

    QString cmd = m_version->stoprobot.at( 0).trimmed();
    int ix = cmd.indexOf( ' ');
    if ( ix < 0 ) return;

    if ( cmd.left( ix).toUpper() == "SEND" ) {
        cmd = cmd.right( cmd.length() - ix - 1).trimmed();
        if ( !serialOpen() ) return;
        serialWrite( cmd);
    }
    else
    if ( cmd.left( ix).toUpper() == "KILL" ) {
        killProtoItRun();
    }
}

void MainWindow::on_actionSetTime_triggered()
{
    if ( !serialOpen() ) return;
    if ( !m_serial.isOpen() ) goto timefailure;
    if ( !serialWrite( "RTCMDTIME") ) goto timefailure;
    if ( !serialWrite( QTime::currentTime().toString( "hh:mm:ss")) ) goto timefailure;
    message( QMessageBox::Information, tr( "De huidige tijd is naar de robot verzonden."));
    return;
timefailure:
    message( QMessageBox::Warning, tr( "De robot heeft de huidige tijd niet ontvangen."));
}

void MainWindow::on_actionMonitor_triggered()
{
    if ( !serialOpen() ) return;
    SerialMonitor sermon( this);
    sermon.exec();
}

void MainWindow::on_butToBack_clicked()
{
    int ix = ui->stepTab->currentIndex();

    // config-tab, final step and add-tab may not be replaced
    if ( ix < 1 || ix >= ui->stepTab->count() - 2 )
        return;

    g_history.stepMoved( ix, ix + 1);
    moveStep( ix, ix + 1);
}

void MainWindow::on_butToFront_clicked()
{
    int ix = ui->stepTab->currentIndex();

    // config-tab, first step and add-tab may not be replaced
    if ( ix < 2 || ix >= ui->stepTab->count() - 1 )
        return;

    g_history.stepMoved( ix, ix - 1);
    moveStep( ix, ix - 1);
}

void MainWindow::moveStep( int ixfrom, int ixto)
{
    m_movestep = true;
    Step *step = (Step*) ui->stepTab->widget( ixfrom);
    QString txt = ui->stepTab->tabText( ixfrom);
    ui->stepTab->removeTab( ixfrom);
    ui->stepTab->insertTab( ixto, step, txt);
    ui->stepTab->setCurrentIndex( ixto);
    renumberSteps();
    m_movestep = false;
}

void MainWindow::on_actionPlatformManager_triggered()
{
    PlatformManager pm( this);
    pm.exec();
}
