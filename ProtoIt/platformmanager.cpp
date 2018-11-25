#include "platformmanager.h"
#include "ui_platformmanager.h"
#include <QDir>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QStandardPaths>

#define TAG_VERSION     0
#define TAG_PLATFORM    1
#define TAG_PROGRAMMER  2
#define TAG_DEVICE      3

#define DEV_MODEL       0
#define DEV_DEVICE      1
#define DEV_FUNCTION    2

const QString VERSIONS = "versions/";
const QString PLATFORMS = "platforms/";
const QString PROGRAMMERS = "programmers/";

#define OLD 0
#define NEW 1

QString NOTFULLYREMOVED;
QString NOTFULLYINSTALLED;
QString CANCELED;

PlatformManager::PlatformManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlatformManager)
{
    ui->setupUi(this);
    m_main = (MainWindow*) parent;
    m_pathOld = m_main->pluginPath();
    m_curVersionOld = m_curPlatformOld = m_curProgrOld = -1;
    m_curVersionNew = m_curPlatformNew = m_curProgrNew = -1;
    listVersions( OLD, m_pathOld + VERSIONS);

    NOTFULLYREMOVED = tr( "-- Niet geheel verwijderd --");
    NOTFULLYINSTALLED = tr( "-- Niet geheel geïnstalleerd --");
    CANCELED = tr( "-- Overgeslagen --");
}

PlatformManager::~PlatformManager()
{
    delete ui;
}

void PlatformManager::on_lstVersionOld_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString version, platform, programmer;
    if ( current ) {
        current->setTextColor( QColor( "blue"));
        current->setFont( QFont( "Segoe UI", 9, 400, true));
    }
    if ( previous ) {
        previous->setTextColor( QColor( "black"));
        previous->setFont( QFont( "Segoe UI"));
    }
    int ix = m_curVersionOld = ui->lstVersionOld->currentRow();
    if ( ix >= 0 && ix < m_versionOld.count() ) {
        QString versionfile = m_pathOld + VERSIONS + m_versionOld.at( ix) + "/" + m_versionOld.at( ix) + ".cfg";
        readConfig( version, platform, programmer, versionfile);
        m_curPlatformOld = m_platformOld.indexOf( platform);
        m_curProgrOld = m_progrOld.indexOf( programmer);
    }
    else
        m_curVersionOld = m_curPlatformOld = m_curProgrOld = -1;

    ui->edtPlatformOld->setText( platform);
    ui->edtProgrOld->setText( programmer);

    listDevices( ui->lstDeviceOld, m_pathOld + PLATFORMS + platform + "/Devices/");
}

void PlatformManager::on_lstVersionNew_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    QString version, platform, programmer;
    if ( current ) {
        current->setTextColor( QColor( "blue"));
        current->setFont( QFont( "Segoe UI", 9, 400, true));
    }
    if ( previous ) {
        previous->setTextColor( QColor( "black"));
        previous->setFont( QFont( "Segoe UI"));
    }
    int ix = m_curVersionNew = ui->lstVersionNew->currentRow();
    if ( ix >= 0 && ix < m_versionNew.count() ) {
        QString versionfile = m_pathNew + VERSIONS + m_versionNew.at( ix) + "/" + m_versionNew.at( ix) + ".cfg";
        readConfig( version, platform, programmer, versionfile);
        m_curPlatformNew = m_platformNew.indexOf( platform);
        m_curProgrNew = m_progrNew.indexOf( programmer);
    }
    else
        m_curVersionNew = m_curPlatformNew = m_curProgrNew = -1;

    ui->edtPlatformNew->setText( platform);
    ui->edtProgrNew->setText( programmer);

    listDevices( ui->lstDeviceNew, m_pathNew + PLATFORMS + platform + "/Devices/");
}

void PlatformManager::on_butGetNewVersions_clicked()
{
    QString mydocs = QStandardPaths::writableLocation( QStandardPaths::DocumentsLocation);
    QString dir = QFileDialog::getExistingDirectory( NULL, "Map met nieuwe/gewijzigde onderdelen", mydocs,
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    if ( dir.isEmpty() ) return; // user canceled the request
    dir += "/";
    if ( dir.right( 10).toLower() ==  "/" + VERSIONS )
        dir = dir.left( dir.length() - 10);
    else
    if ( dir.right( 11).toLower() == "/" + PLATFORMS )
        dir = dir.left( dir.length() - 11);
    else
    if ( dir.right( 13).toLower() == "/" +PROGRAMMERS )
        dir = dir.left( dir.length() - 13);
    QDir src;
    if ( !src.exists( dir + VERSIONS) || !src.exists( dir + PLATFORMS) || !src.exists( dir + PROGRAMMERS) ) {
        m_main->message( QMessageBox::Warning, "De juiste indeling van mappen met benodigde onderdelen is niet gevonden.\n\nEr ontbreekt een map met de volgende drie submappen:\n- platforms\n- programmers\n- versions");
        return;
    }

    m_pathNew = dir;
    listVersions( NEW, dir + VERSIONS);
}

QStringList PlatformManager::listDirs( QString path)
{
    QDir dir;
    dir.setPath( path);
    return dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot);
}

void PlatformManager::readConfig( QString& version, QString& platform, QString& programmer, QString versionfile)
{
    QFile file( versionfile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    version = platform = programmer = "";

    QTextStream in( &file);

    QString ln;
    int     tag = -1;

    while ( !in.atEnd() ) {
        ln = in.readLine().trimmed();
        if ( ln.isEmpty() ) continue;

        if ( ln.toUpper() == "[VERSION]") tag = TAG_VERSION;
        else
        if ( ln.toUpper() == "[PLATFORM]") tag = TAG_PLATFORM;
        else
        if ( ln.toUpper() == "[PROGRAMMER]") tag = TAG_PROGRAMMER;
        else {
            switch ( tag ) {
            case TAG_VERSION :      version = ln; break;
            case TAG_PLATFORM :     platform = ln; break;
            case TAG_PROGRAMMER :   programmer = ln; break;
            }
        }
        if ( !version.isEmpty() && !platform.isEmpty() && !programmer.isEmpty() )
            break;
    }

    file.close();
}

void PlatformManager::listVersions( int type, QString path)
{
    QString version, platform, programmer;
    switch ( type ) {

        case OLD:
            m_versionOld = listDirs( path);
            for ( int i = 0; i < m_versionOld.count(); i++ ) {
                QString versionfile = path + m_versionOld.at( i) + "/" + m_versionOld.at( i) + ".cfg";
                readConfig( version, platform, programmer, versionfile);
                ui->lstVersionOld->addItem( version);
                m_platformOld.append( platform);
                m_progrOld.append( programmer);
            }
            ui->lstVersionOld->setCurrentRow( 0);
            break;

        case NEW:
            m_versionNew = listDirs( path);
            for ( int i = 0; i < m_versionNew.count(); i++ ) {
                QString versionfile = path + m_versionNew.at( i) + "/" + m_versionNew.at( i) + ".cfg";
                readConfig( version, platform, programmer, versionfile);
                ui->lstVersionNew->addItem( version);
                m_platformNew.append( platform);
                m_progrNew.append( programmer);
            }
            ui->lstVersionNew->setCurrentRow( 0);
            break;
    }
}

void PlatformManager::listDevices( QListWidget* list, QString path)
{
    QStringList      devs;
    QListWidgetItem* lwi;
    QList<int>*      ldt;

    list->clear();
    ldt = ( list == ui->lstDeviceNew ? &m_devTypeNew : &m_devTypeOld);
    ldt->clear();

    devs = listDirs( path + "Models");
    if ( !devs.isEmpty() ) {
        list->addItem( tr( "MODELLEN:"));
        lwi = list->item( list->count() - 1);
        lwi->setTextColor( QColor( "blue"));
        lwi->setFont( QFont( "Segoe UI", 9, 400, true));
        list->addItems( devs);
        list->addItem( "");

        ldt->append( -1);
        for ( int i = 0; i < devs.count(); i++ )
            ldt->append( DEV_MODEL);
        ldt->append( -1);
    }

    devs = listDirs( path + "Devices");
    if ( !devs.isEmpty() ) {
        list->addItem( tr( "APPARATEN:"));
        lwi = list->item( list->count() - 1);
        lwi->setTextColor( QColor( "blue"));
        lwi->setFont( QFont( "Segoe UI", 9, 400, true));
        list->addItems( devs);
        list->addItem( "");

        ldt->append( -1);
        for ( int i = 0; i < devs.count(); i++ )
            ldt->append( DEV_DEVICE);
        ldt->append( -1);
    }

    devs = listDirs( path + "Functions");
    if ( !devs.isEmpty() ) {
        list->addItem( tr( "FUNCTIES:"));
        lwi = list->item( list->count() - 1);
        lwi->setTextColor( QColor( "blue"));
        lwi->setFont( QFont( "Segoe UI", 9, 400, true));
        list->addItems( devs);

        ldt->append( -1);
        for ( int i = 0; i < devs.count(); i++ )
            ldt->append( DEV_FUNCTION);
    }
}

void PlatformManager::reportDeletions()
{
    if ( m_updates.isEmpty() )
        m_main->message( QMessageBox::Warning, tr( "Er is een fout opgetreden.\nEr is niets verwijderd."));
    else {
        QString msg = tr( "Het volgende is verwijderd:\n");
        for ( int i = 0; i < m_updates.count(); i++ )
            msg += "\n" + m_updates.at( i);
        m_main->message( QMessageBox::Information, msg);
    }
}

void PlatformManager::reportInstallations()
{
    if ( m_updates.isEmpty() )
        m_main->message( QMessageBox::Warning, tr( "Er is een fout opgetreden.\nEr is niets geïnstalleerd."));
    else {
        QString msg = tr( "Het volgende is geïnstalleerd:\n");
        for ( int i = 0; i < m_updates.count(); i++ )
            msg += "\n" + m_updates.at( i);
        m_main->message( QMessageBox::Information, msg);
    }
}

void PlatformManager::on_butDelVersion_clicked()
{
    m_updates.clear();

    if ( m_curVersionOld < 0 ) {
        m_main->message( QMessageBox::Information, tr( "Selecteer eerst een versie."));
        return;
    }
    QString version = m_versionOld.at( m_curVersionOld);
    QString msg = tr( "Het volgende wordt hiermee verwijderd:\n") +
                  tr( "- de versie zelf\n") +
                  tr( "- het platform als er geen andere versie gebruik van maakt\n") +
                  tr( "   (ook alle bijbehorende onderdelen)\n") +
                  tr( "- de programmer als er geen andere versie gebruik van maakt");
    QString ask = tr( "Versie '") + version + tr( "' echt verwijderen?");
    QString str;
    if ( m_main->message( QMessageBox::Question, msg, ask) == QMessageBox::Yes ) {
        str = m_platformOld.at( m_curPlatformOld);
        if ( m_platformOld.indexOf( str) == m_platformOld.lastIndexOf( str) ) {
            deleteType( TAG_PLATFORM, m_pathOld + PLATFORMS + m_platformOld.at( m_curPlatformOld) + "/");
        }
        str = m_progrOld.at( m_curProgrOld);
        if ( m_progrOld.indexOf( str) == m_progrOld.lastIndexOf( str) ) {
            deleteType( TAG_PROGRAMMER, m_pathOld + PROGRAMMERS + m_progrOld.at( m_curProgrOld) + "/");
        }
        deleteType( TAG_VERSION, m_pathOld + VERSIONS + version + "/");
        reportDeletions();

        ui->lstVersionOld->clear();
        m_versionOld.clear();
        m_platformOld.clear();
        m_progrOld.clear();
        ui->lstDeviceOld->clear();
        m_devTypeOld.clear();
        listVersions( OLD, m_pathOld + VERSIONS);
        ui->lstVersionOld->setCurrentRow( 0);

        m_main->loadVersions();
        m_main->setVersion( 0, false);
    }
}

void PlatformManager::on_butDelDevice_clicked()
{
    m_updates.clear();

    QListWidgetItem* lwi = ui->lstDeviceOld->currentItem();
    if ( !lwi || lwi->text().isEmpty() ) {
        m_main->message( QMessageBox::Information, tr( "Selecteer eerst een onderdeel."));
        return;
    }

    QString device = lwi->text();
    if ( device == tr( "MODELLEN:") || device == tr( "APPARATEN:") || device == tr( "FUNCTIES:") ) {
        m_main->message( QMessageBox::Information, "'" + device + tr( "' is een titel en geen onderdeel."));
        return;
    }
    QString msg = tr( "Het volgende wordt hiermee verwijderd:\n") +
                  tr( "- alleen het betreffende onderdeel\n\n") +
                  tr( "LET OP!\nHet onderdeel wordt uit alle versies verwijderd,\ndie van het bijbehorende platform gebruik maken.");
    QString ask = tr( "Onderdeel '") + device + tr( "' echt verwijderen?");
    if ( m_main->message( QMessageBox::Question, msg, ask) == QMessageBox::Yes ) {
        QString devname;
        int devtype = m_devTypeOld.at( ui->lstDeviceOld->currentRow());
        switch ( devtype ) {
            case DEV_MODEL : devname = "Models/"; break;
            case DEV_DEVICE : devname = "Devices/"; break;
            case DEV_FUNCTION : devname = "Functions/"; break;
        }

        QString sffx = PLATFORMS + m_platformOld.at( m_curPlatformOld) + "/Devices/" + devname + device;
        deleteType( TAG_DEVICE, m_pathOld + sffx + "/");
        reportDeletions();

        m_devTypeOld.clear();
        listDevices( ui->lstDeviceOld,  m_pathOld + PLATFORMS + m_platformOld.at( m_curPlatformOld) + "/Devices/");
        ui->lstDeviceOld->setCurrentRow( 1);
        ui->lstDeviceOld->setFocus();
    }
}

void PlatformManager::on_butAddVersion_clicked()
{
    m_updates.clear();

    if ( m_curVersionNew < 0 ) {
        m_main->message( QMessageBox::Information, tr( "Selecteer eerst een versie."));
        return;
    }
    QString version = m_versionNew.at( m_curVersionNew);
    QString msg = tr( "Het volgende wordt hiermee geïnstalleerd:\n") +
                  tr( "- de versie zelf\n") +
                  tr( "- het bijbehorende platform\n   (inclusief onderdelen)\n") +
                  tr( "- de bijbehorende programmer\n\n") +
                  tr( "Indien van toepassing wordt vooraf gevraagd of een eerdere installatie moet worden overschreven.");
    QString ask = tr( "Versie '") + version + tr( "' nu installeren?");
    if ( m_main->message( QMessageBox::Question, msg, ask) == QMessageBox::Yes ) {
        QString sffx;
        sffx = VERSIONS + version + "/";
        copyType( TAG_VERSION, m_pathNew + sffx, m_pathOld + sffx);
        sffx = PLATFORMS + m_platformNew.at( m_curPlatformNew) + "/";
        copyType( TAG_PLATFORM, m_pathNew + sffx, m_pathOld + sffx);
        sffx = PROGRAMMERS + m_progrNew.at( m_curProgrNew) + "/";
        copyType( TAG_PROGRAMMER, m_pathNew + sffx, m_pathOld + sffx);
        reportInstallations();

        ui->lstVersionOld->clear();
        m_versionOld.clear();
        m_platformOld.clear();
        m_progrOld.clear();
        ui->lstDeviceOld->clear();
        m_devTypeOld.clear();
        listVersions( OLD, m_pathOld + VERSIONS);
        int i;
        for ( i = 0; i < m_versionOld.count(); i++ )
            if ( m_versionOld.at( i) == version )
                break;
        if ( i >= ui->lstVersionOld->count() )
            i = 0;
        ui->lstVersionOld->setCurrentRow( i);

        m_main->loadVersions();
        i = m_main->indexOfVersion( version);
        m_main->setVersion( i >= 0 ? i : 0);
    }
}

void PlatformManager::on_butAddDevice_clicked()
{
    m_updates.clear();

    QListWidgetItem* lwi = ui->lstDeviceNew->currentItem();
    if ( !lwi || lwi->text().isEmpty() ) {
        m_main->message( QMessageBox::Information, tr( "Selecteer eerst een onderdeel."));
        return;
    }

    QString version = m_versionNew.at( m_curVersionNew);
    int oldversion;
    for ( oldversion = 0; oldversion < m_versionOld.count(); oldversion++ )
        if ( m_versionOld.at( oldversion) == version )
            break;
    if ( oldversion >= m_versionOld.count() ) {
        version = ui->lstVersionNew->currentItem()->text();
        m_main->message( QMessageBox::Information, tr( "Het onderdeel hoort bij een versie, die nog niet is geïnstalleerd.\n\n") +
                                                   tr( "Installeer eerst versie '") + version + "'.");
        return;
    }

    QString device = lwi->text();
    if ( device == tr( "MODELLEN:") || device == tr( "APPARATEN:") || device == tr( "FUNCTIES:") ) {
        m_main->message( QMessageBox::Information, "'" + device + tr( "' is een titel en geen onderdeel."));
        return;
    }
    QString msg = tr( "Het volgende wordt hiermee geïnstalleerd:\n") +
                  tr( "- alleen het betreffende onderdeel\n\n") +
                  tr( "Indien van toepassing wordt vooraf gevraagd of een eerdere installatie moet worden overschreven.\n\n") +
                  tr( "LET OP!\nHet onderdeel wordt aan alle versies toegevoegd, die van het bijbehorende platform gebruik maken.");
    QString ask = tr( "Onderdeel '") + device + tr( "' nu installeren?");
    if ( m_main->message( QMessageBox::Question, msg, ask) == QMessageBox::Yes ) {
        QString sffx;
        QString devname;

        int devtype = m_devTypeNew.at( ui->lstDeviceNew->currentRow());
        switch ( devtype ) {
            case DEV_MODEL : devname = "Models/"; break;
            case DEV_DEVICE : devname = "Devices/"; break;
            case DEV_FUNCTION : devname = "Functions/"; break;
        }
/*
 * version, platform and programmer don't get updated automatically any longer
 * in the above code the version got checked on existance already

        sffx = VERSIONS + m_versionNew.at( m_curVersionNew) + "/";
        if ( !dir.exists( m_pathOld + sffx) )
            copyType( TAG_VERSION, m_pathNew + sffx, m_pathOld + sffx);
        sffx = PLATFORMS + m_platformNew.at( m_curPlatformNew) + "/";
        if ( !dir.exists( m_pathOld + sffx) )
            dir.mkpath( m_pathOld + sffx);
        sffx = PROGRAMMERS + m_progrNew.at( m_curProgrNew) + "/";
        if ( !dir.exists( m_pathOld + sffx) )
            copyType( TAG_PROGRAMMER, m_pathNew + sffx, m_pathOld + sffx);
*/

        sffx = PLATFORMS + m_platformNew.at( m_curPlatformNew) + "/Devices/" + devname + device;
        copyType( TAG_DEVICE, m_pathNew + sffx + "/", m_pathOld + sffx + "/");
        sffx += ".rtd";
        QFile::copy( m_pathNew + sffx, m_pathOld + sffx);
        reportInstallations();

        m_curVersionOld = oldversion;
        ui->lstVersionOld->setCurrentRow( oldversion);
        listDevices( ui->lstDeviceOld, m_pathOld + PLATFORMS + m_platformOld.at( m_curPlatformOld) + "/Devices/");
        int i;
        for ( i = 0; i < ui->lstDeviceOld->count(); i++ )
            if ( ui->lstDeviceOld->item( i)->text() == device )
                break;
        if ( i >= ui->lstDeviceOld->count() )
            i = 1;
        ui->lstDeviceOld->setCurrentRow( i);
        ui->lstDeviceOld->setFocus();
    }
}

void PlatformManager::deleteType( int type, QString path)
{
    QString name, tag, str;
    str = path.left( path.length() - 1); // remove final '/'
    name = str.right( str.length() - str.lastIndexOf( '/') - 1);
    switch ( type ) {
    case TAG_PLATFORM : tag = tr( "Platform '") + name + "'"; break;
    case TAG_PROGRAMMER : tag = tr( "Programmer '") + name + "'"; break;
    case TAG_VERSION : tag = tr( "Versie '") + name + "'"; break;
    case TAG_DEVICE : tag = tr( "Onderdeel '") + name + "'"; break;
    }
    m_updates.append( tag);
    QDir dir( path);
    if ( !dir.removeRecursively() )
        m_updates.append( NOTFULLYREMOVED);
    if ( type == TAG_DEVICE ) {
        str += ".rtd";
        if ( !QFile::remove( str) )
            m_updates.append( NOTFULLYREMOVED);
    }
}

void PlatformManager::copyType( int type, QString srcpath, QString dstpath)
{
    QString name, tag, str;
    str = srcpath.left( srcpath.length() - 1); // remove final '/'
    name = str.right( str.length() - str.lastIndexOf( '/') - 1);
    switch ( type ) {
    case TAG_PLATFORM : tag = tr( "Platform '") + name + "'"; break;
    case TAG_PROGRAMMER : tag = tr( "Programmer '") + name + "'"; break;
    case TAG_VERSION : tag = tr( "Versie '") + name + "'"; break;
    case TAG_DEVICE : tag = tr( "Onderdeel '") + name + "'"; break;
    }
    m_updates.append( tag);

    QDir dir( dstpath);
    bool overwrite = false;
    if ( dir.exists() ) {
        int ret = m_main->message( QMessageBox::Warning, tag + " bestaat al.",
                                   "Overschrijven met de nieuwe versie?");
        if ( ret == QMessageBox::Cancel ) {
            m_updates.append( CANCELED);
            return;
        }
        overwrite = (ret == QMessageBox::Yes);
    }
    if ( !copyRecursively( srcpath, dstpath, overwrite) )
        m_updates.append( NOTFULLYINSTALLED);
}

/*
bool PlatformManager::deleteRecursively( QString path)
{
    bool ret = true;
    int i;
    QDir dir( path);
    QStringList files = dir.entryList( "*", QDir::Files);
    QStringList dirs = dir.entryList( "*", QDir::Dirs | QDir::NoDotAndDotDot);
    for ( i = 0; i < files.count(); i++ )
        if ( !QFile::remove( path + files.at( i)) }
            ret = false;
    for ( i = 0; i < dirs.count(); dir++ )
        if ( !deleteDir( path + dir.at( i)) )
            ret = false;
    if ( !QDir::remove( path) )
        ret = false;
    return ret;
}
*/

bool PlatformManager::copyRecursively( QString srcpath, QString dstpath, bool overwrite)
{
    bool ret = true;
    int i;
    QFile file;
    QDir srcdir( srcpath);
    QDir dstdir( dstpath);
    QStringList files = srcdir.entryList( QDir::Files);
    QStringList dirs = srcdir.entryList( QDir::Dirs | QDir::NoDotAndDotDot);
    if ( dstdir.mkpath( dstpath) ) {
        for ( i = 0; i < files.count(); i++ ) {
            file.setFileName( dstpath + files.at( i));
            if ( overwrite ) file.remove();
            if ( !file.exists() )
                if ( !QFile::copy( srcpath + files.at( i), dstpath + files.at( i)) )
                    ret = false;
        }
        for ( i = 0; i < dirs.count(); i++ )
            if ( !copyRecursively( srcpath + dirs.at( i) + "/", dstpath + dirs.at( i) + "/", overwrite) )
                ret = false;
    }
    else
        ret = false;

    return ret;
}
