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

#ifndef CODEWRITER_H
#define CODEWRITER_H

#include <QTextStream>
#include <QCoreApplication>

class MainWindow;
class Tile;
class Field;

/*
 * CODEWRITER class
 * contains the code of a device template (comes from file .rtd)
 */

class CodeWriter
{
    Q_DECLARE_TR_FUNCTIONS(CodeWriter)

public:

    QStringList      m_copyright;  // code writer copyrights
    QStringList      m_include;    // code writer '#include'
    QStringList      m_global;     // code writer for global declarations
    QStringList      m_local;      // code writer for local declarations in the main loop
    QStringList      m_routine;    // code writer for routines
    QStringList      m_setup;      // code writer for setup routine
    QStringList      m_init;       // code writer for loop part 1: step initialisation
    QStringList      m_sud;        // code writer for loop part 2: sensor update
    QStringList      m_fud;        // code writer for loop part 3: function handling
    QStringList      m_aud;        // code writer for loop part 4: activator update

    CodeWriter( MainWindow* main);

    void tabs( int n);

    QString writeCopyright( QString device, QStringList& copyrights);
    QString writeInclude( QStringList& includes);
    QString writeGlobal();
    QString writeLocal();
    QString writeRoutine();
    QString writeSetup();
    QString writeStepInit();
    QString writeSensorUpdate();
    QString writeFunctionUpdate();
    QString writeActuatorUpdate();
    QString writeDataFeedthrough( QString setvariant, Tile* tile);

    void replaceKeys( QString key, QString tilekey);

protected:

    QString replace( QString line, QString before, QString after);

    MainWindow* m_main;
    QString     m_tabs;
};

#endif // CODEWRITER_H
