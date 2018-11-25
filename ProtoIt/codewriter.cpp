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

#include "codewriter.h"
#include "mainwindow.h"
#include "tile.h"
#include "field.h"

CodeWriter::CodeWriter( MainWindow* main)
{
    m_main = main;
    m_tabs = "    ";
}

void CodeWriter::tabs( int n)
{
    m_tabs = "";
    for ( int i = 0; i < n; i++ )
        m_tabs += '  ';
}

QString CodeWriter::writeCopyright( QString device, QStringList& copyrights)
{
    QString code;
    for ( int i = 0; i < m_copyright.count(); i++ ) {
        if ( copyrights.contains( m_copyright.at( i)) )
            continue;
        code += tr( "// Voor apparaat ") + m_copyright.at( i) + "\n";
    }
    return code;
}

QString CodeWriter::writeInclude( QStringList& includes)
{
    QString code;
    for ( int i = 0; i < m_include.count(); i++ ) {
        if ( includes.contains( m_include.at( i)) )
            continue;
        code += m_include.at( i) + "\n";
    }
    return code;
}

QString CodeWriter::writeGlobal()
{
    QString code;
    for ( int i = 0; i < m_global.count(); i++ )
        code += m_global.at( i) + "\n";
    return code;
}

QString CodeWriter::writeRoutine()
{
    QString code;
    for ( int i = 0; i < m_routine.count(); i++ )
        code += m_routine.at( i) + "\n";
    return code;
}

QString CodeWriter::writeSetup()
{
    QString code;
    for ( int i = 0; i < m_setup.count(); i++ )
        code += m_tabs + m_setup.at( i) + "\n";
    return code;
}

QString CodeWriter::writeStepInit()
{
    QString code;
    for ( int i = 0; i < m_init.count(); i++ )
        code += m_tabs + m_init.at( i) + "\n";
    return code;
}

QString CodeWriter::writeSensorUpdate()
{
    QString code;
    for ( int i = 0; i < m_sud.count(); i++ )
        code += m_tabs + m_sud.at( i) + "\n";
    return code;
}

QString CodeWriter::writeFunctionUpdate()
{
    QString code;
    for ( int i = 0; i < m_fud.count(); i++ )
        code += m_tabs + m_fud.at( i) + "\n";
    return code;
}

QString CodeWriter::writeActuatorUpdate()
{
    QString code;
    for ( int i = 0; i < m_aud.count(); i++ )
        code += m_tabs + m_aud.at( i) + "\n";
    return code;
}

QString CodeWriter::writeDataFeedthrough( QString setvariant, Tile* tile)
{
    QString code;
    Field* field = tile->firstField();
    QString signalkey,stepnum, value, outstr;
    while ( field ) {
        value = field->value().trimmed();
        if ( !value.isEmpty() ) {
            signalkey = m_main->findSignalKey( value);          // try if value contains a signal key
            if ( signalkey.isEmpty() ) {
                    stepnum = m_main->findStepNumber( value);   // try if value contains a step name
                if ( !stepnum.isEmpty() )
                    value = stepnum;
                outstr = setvariant;
                outstr.replace( "%variant%", field->key());
                outstr.replace( "%value%", value);
            }
            else
                outstr = field->key() + " = " + signalkey + ";";
            code += m_tabs + outstr + "\n";
        }
        field = tile->nextField();
    }
    return code;
}

void CodeWriter::replaceKeys( QString key, QString tilekey)
{
    int i;
    QString s;

    for ( i = 0; i < m_global.count(); i++ ) {
        s = m_global.at( i);
        s = replace( s, key, tilekey);
        m_global.replace( i, s);
    }
    for ( i = 0; i < m_local.count(); i++ ) {
        s = m_local.at( i);
        s = replace( s, key, tilekey);
        m_local.replace( i, s);
    }
    for ( i = 0; i < m_routine.count(); i++ ) {
        s = m_routine.at( i);
        s = replace( s, key, tilekey);
        m_routine.replace( i, s);
    }
    for ( i = 0; i < m_setup.count(); i++ ) {
        s = m_setup.at( i);
        s = replace( s, key, tilekey);
        m_setup.replace( i, s);
    }
    for ( i = 0; i < m_init.count(); i++ ) {
        s = m_init.at( i);
        s = replace( s, key, tilekey);
        m_init.replace( i, s);
    }
    for ( i = 0; i < m_sud.count(); i++ ) {
        s = m_sud.at( i);
        s = replace( s, key, tilekey);
        m_sud.replace( i, s);
    }
    for ( i = 0; i < m_fud.count(); i++ ) {
        s = m_fud.at( i);
        s = replace( s, key, tilekey);
        m_fud.replace( i, s);
    }
    for ( i = 0; i < m_aud.count(); i++ ) {
        s = m_aud.at( i);
        s = replace( s, key, tilekey);
        m_aud.replace( i, s);
    }
}

QString CodeWriter::replace( QString line, QString before, QString after)
{
    QString special = " ()[].,=-+*/<>|&^%$#!~\"':;?\t\r\n";
    int il, ib, ie;

    il = 0;
    while ( il < line.length() ) {
        ie = il;
        for ( ib = 0; ib < before.length() && ie < line.length(); ib++, ie++ )
            if ( line.at( ie) != before.at( ib) ) break;
        if ( (ib == before.length()) && (ie < line.length()) && special.contains( line.at( ie))  ) {
            // when a word fits, replace it
            line = line.left( il) + after + line.right( line.length() - ie);
            il += after.length();
        }
        else {
            // when a word doesn't fit, skip it
            while ( il < line.length() && !special.contains( line.at( il)) )
                il++;
         }
        // skip to the next word
        while ( (il < line.length()) && special.contains( line.at( il)) )
            il++;
    }
    return line;
}
