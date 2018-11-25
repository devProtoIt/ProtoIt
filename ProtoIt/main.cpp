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
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

int main( int argc, char *argv[])
{
    QApplication app( argc, argv);

    QTranslator qtts;
    qtts.load( "qt_uk");
    app.installTranslator( &qtts);

    QTranslator appts;
    appts.load( "ProtoIt_en");
    app.installTranslator(&appts);

    MainWindow w;
    w.showMaximized();

    return app.exec();
}
