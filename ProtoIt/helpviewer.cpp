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

#include "helpviewer.h"
#include "ui_helpviewer.h"
#include <qdesktopwidget.h>
#include <qfile.h>

HelpViewer::HelpViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpViewer)
{
    ui->setupUi(this);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    ui->wwwHelp->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
}

HelpViewer::~HelpViewer()
{
    delete ui;
}

void HelpViewer::setHelp( QString filepath)
{
    QString path = filepath.left( filepath.lastIndexOf( '/') + 1);
    QFile file( filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in( &file);
    QStringList line;
    QString     ln;

    while ( !in.atEnd() ) {
        ln = in.readLine().trimmed();
        if ( ln.left( 2) == "//" ) continue;
        if ( ln.isEmpty() ) {
            ui->lstHelp->addItem( "");
            m_help.append( "");
        }
        line = ln.split( "==");
        if ( line.count() != 2 ) continue;
        ui->lstHelp->addItem( line.first().trimmed());
        m_help.append( path + line.last().trimmed());
    }

    file.close();

    // initialize the window
    if ( m_help.count() ) {
        m_page = m_help.at( 0);
        ui->wwwHelp->setUrl( QUrl::fromLocalFile( m_page));
    }
}

void HelpViewer::on_lstHelp_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    m_page = m_help.at( ui->lstHelp->currentRow());
    ui->wwwHelp->setUrl( QUrl::fromLocalFile( m_page));
}

void HelpViewer::on_btnSearch_clicked()
{
    ui->wwwHelp->findText( ui->edtSearch->text(), QWebPage::HighlightAllOccurrences);
}

void HelpViewer::on_btnClear_clicked()
{
    ui->wwwHelp->setUrl( QUrl::fromLocalFile( m_page));
}
