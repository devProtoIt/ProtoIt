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

#include "template.h"
#include <QAction>
#include <QLineEdit>
#include <QComboBox>
#include <QDir>
#include "mainwindow.h"
#include "field.h"

/*
 * CLASS VARLIST
 */

VarList::~VarList()
{
    for ( int i = 0; i < m_decl.count(); i++ )
        delete m_decl.at( i);
}

void VarList::addVar( Var* var)
{
    m_decl.append( var);
}

Var* VarList::firstVar( VarType vt)
{
    for ( m_ix = 0; m_ix < m_decl.count(); m_ix++ )
        if ( m_decl.at( m_ix)->vt & vt )
            return ( m_decl.at( m_ix));
    return NULL;
}

Var* VarList::nextVar( VarType vt)
{
    for ( m_ix++; m_ix < m_decl.count(); m_ix++ )
        if ( m_decl.at( m_ix)->vt & vt )
            return ( m_decl.at( m_ix));
    return NULL;
}

/*
 * CLASS TEMPLATE
 */

Template::Template( MainWindow* main)
{
    m_main = main;
    m_pt = ptDevice;
    m_tile = NULL;
    m_code = new CodeWriter( main);
}

void Template::setPath( QString path)
{
    m_path = path;
}

void Template::setFilename( QString filename)
{
    m_filename = filename;
}

void Template::setAlias( QString alias)
{
    m_alias = alias;
}

void Template::parseLine( QString line, QString platform, QString model)
{
    QString str, trimmed = line.trimmed();
    QStringList list;
    Var* var;

    if ( trimmed == "<DEVICE>" ) m_pt = ptDevice;
    else if ( trimmed == "<ALIAS>" ) m_pt = ptAlias;
    else if ( trimmed == "<ICON>" ) m_pt = ptIcon;
    else if ( trimmed == "<PORT>" ) m_pt = ptPort;
    else if ( trimmed == "<ADDRESS>" ) m_pt = ptAddress;
    else if ( trimmed == "<PRIVATE>" ) m_pt = ptPrivate;
    else if ( trimmed == "<CALIBRATE>" ) m_pt = ptCalibrate;
    else if ( trimmed == "<SIGNALIN>" ) m_pt = ptSignalIn;
    else if ( trimmed == "<SIGNALOUT>" ) m_pt = ptSignalOut;
    else if ( trimmed == "<CONSTANT>") m_pt = ptConstant;
    else if ( trimmed == "<COPYRIGHT>" ) m_pt = ptCopyright;
    else if ( trimmed == "<INCLUDE>" ) m_pt = ptInclude;
    else if ( trimmed == "<LIBRARY>" ) m_pt = ptLibrary;
    else if ( trimmed == "<GLOBAL>" ) m_pt = ptGlobal;
    else if ( trimmed == "<ROUTINE>" ) m_pt = ptRoutine;
    else if ( trimmed == "<STEPINIT>") m_pt = ptStepInit;
    else if ( trimmed == "<SETUP>" ) m_pt = ptSetup;
    else if ( trimmed == "<SENSORUPDATE>" ) m_pt = ptSensorUpdate;
    else if ( trimmed == "<FUNCTIONUPDATE>" ) m_pt = ptFunctionUpdate;
    else if ( trimmed == "<ACTUATORUPDATE>" ) m_pt = ptActuatorUpdate;
    else {
        switch ( m_pt ) {
        case ptDevice : list = trimmed.split( ',');
                        if ( list.count() < 2 ) return;
                        m_device = list.at( 1).trimmed();
                        trimmed = list.at( 0).trimmed().toUpper();
                        if ( trimmed == "ACTUATOR" ) m_dt = dtActuator;
                        else if ( trimmed == "SENSOR" ) m_dt = dtSensor;
                        else if ( trimmed == "FUNCTION" ) m_dt = dtFunction;
                        else m_dt = dtUndefined;
                        break;
        case ptAlias : m_alias = trimmed; break;
        case ptIcon : { QDir dir;
                      if ( trimmed.left( 2) == ":/" )
                          m_icon = trimmed.toLower() + ".png";
                      else {
                          m_icon = m_path + model + '/' + trimmed;
                          if ( !dir.exists( m_icon) )
                              m_icon = m_main->homePath() + "Devices/" + platform + '/' + model + '/' + trimmed;
                      }
                      break; }
        case ptPort :
        case ptAddress :
        case ptPrivate :
        case ptCalibrate :
        case ptSignalIn :
        case ptSignalOut :
        case ptConstant :   list = trimmed.split( ',');
                            if ( !list.count() || list.at( 0).isEmpty() ) return;
                            var = new Var;
                            var->key = list.at( 0).trimmed();
                            var->alias = (list.count() > 1 ? list.at( 1).trimmed() : var->key);
                            var->init = "";
                            var->paragraph = "";
                            var->expanded = false;
                            var->vtab = false;
                            if ( list.count() > 2 ) {
                                str = list.at( 2).trimmed().left( 2).toUpper();
                                if ( str == "#P" ) {
                                    var->vtab = true;
                                    str = list.at( 2).trimmed();
                                    var->paragraph = str.right( str.length() - 2).trimmed();
                                    if ( list.count() > 3  && list.at( 3).trimmed().left(2).toUpper() == "#E" )
                                        var->expanded = true;
                                }
                                else
                                if ( str == "#E" ) {
                                    var->expanded = true;
                                    if ( list.count() > 3  && list.at( 3).trimmed().left(2).toUpper() == "#P" ) {
                                        var->vtab = true;
                                        str = list.at( 3).trimmed();
                                        var->paragraph = str.right( str.length() - 2).trimmed();
                                    }
                                }
                                else {
                                    var->init = list.at( 2).trimmed();
                                    if ( list.count() > 3 ) {
                                        str = list.at( 3).trimmed().left( 2).toUpper();
                                        if ( str == "#P" ) {
                                            var->vtab = true;
                                            str = list.at( 3).trimmed();
                                            var->paragraph = str.right( str.length() - 2).trimmed();
                                            if ( list.count() > 4  && list.at( 4).trimmed().left(2).toUpper() == "#E" )
                                                var->expanded = true;
                                        }
                                        else
                                        if ( str == "#E" ) {
                                            var->expanded = true;
                                            if ( list.count() > 4  && list.at( 4).trimmed().left(2).toUpper() == "#P" ) {
                                                var->vtab = true;
                                                str = list.at( 4).trimmed();
                                                var->paragraph = str.right( str.length() - 2).trimmed();
                                            }
                                        }
                                    }
                                }
                           }
                           switch ( m_pt ) {
                           case ptPort :        var->vt = vtPort; break;
                           case ptAddress :     var->vt = vtAddress; break;
                           case ptPrivate :     var->vt = vtPrivate; break;
                           case ptCalibrate :   var->vt = vtCalibrate; break;
                           case ptSignalIn :    var->vt = vtSignalIn; break;
                           case ptSignalOut :   var->vt = vtSignalOut; break;
                           case ptConstant :    var->vt = vtConstant; break;
                           }
                           m_vars.addVar( var);
                           break;
        case ptCopyright : m_code->m_copyright.append( "'" + m_device + "': (C) " + trimmed); break;
        case ptInclude : m_code->m_include.append( trimmed); break;
        case ptLibrary : if ( !m_libs.contains( trimmed, Qt::CaseInsensitive) )
                            m_libs.append( trimmed);
                         break;
        case ptGlobal : var = new Var;
                        var->key = identifier( trimmed);
                        var->alias = var->key;
                        var->init = "";
                        var->vt = vtGlobal;
                        m_vars.addVar( var);
                        m_code->m_global.append( line);
                        break;
        case ptRoutine : m_code->m_routine.append( line); break;
        case ptSetup : m_code->m_setup.append( line); break;
        case ptStepInit : m_code->m_init.append( line); break;
        case ptSensorUpdate : m_code->m_sud.append( line); break;
        case ptFunctionUpdate : m_code->m_fud.append( line); break;
        case ptActuatorUpdate : m_code->m_aud.append( line); break;
        }
    }
}

QString Template::identifier( QString decl)
{
    QString special = "()[].,=-+*/<>|&^%$#!~\"':;?\t\n\r";
    for ( int i = 0; i < special.length(); i++ )
        decl.replace( special.at( i), ' ');
    QStringList res = decl.trimmed().split( ' ', QString::SkipEmptyParts);
    if ( res.count() < 2 )
        return "###";
    return res.at( 1);
}

void Template::replaceKeys()
{
    QString special = " ().,=-+*/<>|&^%$#!~\"':;?";
    QString key;
    Var* var = m_vars.firstVar( vtAll);
    while ( var ) {
        key = m_alias + '_' + var->key;
        for ( int i = 0; i < special.length(); i++ )
            key.replace( special.at( i), '_');
        m_code->replaceKeys( var->key, key);
        var->key = key;
        var = m_vars.nextVar( vtAll);
    }
}

Tile* Template::createDevice()
{
    Step* step = (Step*) m_main->getTab()->widget( 0);

    // create the tile
    m_tile = new Tile( step, this, m_main);
    m_tile->setDevice( m_dt, m_device, m_alias, m_icon);

    Var* var = m_vars.firstVar( vtPort);
    if ( var ) {
        m_tile->createHeader( tr( "Poort:"));
        while ( var ) {
            m_tile->createField( var->key, var->alias, var->init, ftField, false);
            var = m_vars.nextVar( vtPort);
        }
    }

    var = m_vars.firstVar( vtAddress);
    if ( var ) {
        m_tile->createHeader( tr( "Adres:"));
        while ( var ) {
            m_tile->createField( var->key, var->alias, var->init, ftField, false);
            var = m_vars.nextVar( vtAddress);
        }
    }

    var = m_vars.firstVar( vtSignalOut);
    if ( var ) {
        m_tile->createHeader( tr( "Signalen:"));
        while ( var ) {
            m_tile->createField( var->key, var->alias, var->init, ftLabel, false);
            var = m_vars.nextVar( vtSignalOut);
        }
        m_tile->flushField();
    }

    var = m_vars.firstVar( vtCalibrate);
    if ( var ) {
        m_tile->createHeader( tr( "Kalibratie:"));
        while ( var ) {
            m_tile->createField( var->key, var->alias, var->init, ftField, false);
            var = m_vars.nextVar( vtCalibrate);
        }
    }

    var = m_vars.firstVar( vtConstant);
    if ( var ) {
        // no header, since the fields are hidden
        while ( var ) {
            m_tile->createField( var->key, var->alias, var->init, ftConstant, false);
            var = m_vars.nextVar( vtConstant);
        }
    }

    return m_tile;
}

Tile* Template::createTile()
{
    Var* var = m_vars.firstVar( vtSignalIn);

    // prepare
    if ( !var ) {
        m_main->message( QMessageBox::Information, tr( "Dit apparaat zend alleen signalen.\nDe signalen staan in de lijst met beschikbare signalen."));
        return NULL;
    }
    Step* step = (Step*) m_main->getTab()->currentWidget();

    // create the tile
    Tile* tile = new Tile( step, this, m_main,true);
    tile->setDevice( m_dt, m_device, m_alias, m_icon);

    tile->createHeader( tr( "Programma:"));
    while ( var ) {
        if ( !var->paragraph.isEmpty() )
            tile->createHeader( var->paragraph, true);
        tile->createField( var->key, var->alias, var->init, (var->expanded ? ftExpanded : ftField), var->vtab);
        var = m_vars.nextVar( vtSignalIn);
    }

    // add the tile to the step
    tile->buildTile();
    step->appendTile( tile);

    return tile;
}

QString Template::path()
{
    return m_path;
}

QString Template::filename()
{
    return m_filename;
}

QString Template::icon()
{
    return m_icon;
}

QString Template::alias()
{
    return m_alias;
}

QString Template::device()
{
    return m_device;
}

Tile* Template::deviceTile()
{
    return m_tile;
}

DeviceType Template::deviceType()
{
    return m_dt;
}
