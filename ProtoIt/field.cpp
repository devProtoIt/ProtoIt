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

#include "field.h"
#include "ui_field.h"
#include "mainwindow.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include "communicator.h"

Field::Field( MainWindow* main, Tile *tile, QString key, QString alias, int xpos, int ypos, FieldType ft, bool vtab) :
    QWidget( tile),
    ui( new Ui::Field)
{
    ui->setupUi(this);
    setWindowFlags( Qt::FramelessWindowHint);
    m_editlocked = false;
    m_historyon = true;
    m_oldlock = false;
    ui->btnClear->resize( 8, 8);
    ui->btnClear->move( ui->btnClear->x()+5, 7);
    ui->btnClear->hide();
    m_main = main;
    m_tile  = tile;
    m_key = key;
    m_ft = ft;
    m_vtab = vtab;
    ui->label->setText( alias);
    move( xpos, ypos);
    if ( ft == ftField || ft == ftExpanded )
        setAcceptDrops( true);
    if ( ft == ftConstant )
        hide();
    else {
        connect( ui->edit, SIGNAL(editingFinished()), this, SLOT(on_editingFinished()));
        connect( ui->edit, SIGNAL(textChanged(QString)), this, SLOT(on_textChanged(QString)));
        if ( ft == ftLabel )
            ui->edit->hide();
        if ( ft == ftExpanded )
            hide();
        else
            show();
    }
    if ( ft == ftHeader || ft == ftSubhead ) {
        ui->edit->hide();
        ui->label->resize( 240, 20);
        if ( ft == ftSubhead ) {
            ui->label->setStyleSheet( "color:red; font: 800 8pt \"Arial\";");
        }
        else
            ui->label->setStyleSheet( "color:red; font: 800 10pt \"Arial\";");
    }
}

Field::~Field()
{
    delete ui;
}

FieldType Field::type()
{
    return m_ft;
}

QString Field::key()
{
    return m_key;
}

QString Field::alias()
{
    return ui->label->text();
}

QString Field::value()
{
    return ui->edit->text();
}

bool Field::isLocked()
{
    return m_editlocked;
}

bool Field::vTab()
{
    return m_vtab;
}

void Field::initValue( QString value)
{
    m_oldvalue = value;
    setValue( value, true, false);
}

void Field::setValue( QString value, bool unlock, bool historyon)
{
    if ( value == ui->edit->text() )
        return;

    // these changes are recorded in g_history by on_editingFinished
    // unless 'm_historyon' is set to false

    m_historyon = historyon;

    if ( unlock ) {
        m_editlocked = false;
        ui->btnClear->hide();
    }
    if ( value.left( 2) == "> " )
        value = m_tile->alias() + " " + value;
    ui->edit->setText( value);
    m_oldvalue = value;
    if ( value.contains( " > ") ) {
        m_editlocked = true;
        ui->btnClear->show();
    }

    m_historyon = true;
}

void Field::setReadonly( bool readonly)
{
    ui->edit->setReadOnly( readonly);
    ui->btnClear->setEnabled( !readonly);
}

Tile* Field::tile()
{
    return m_tile;
}

void Field::dragEnterEvent( QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void Field::dropEvent( QDropEvent* event)
{
    QByteArray encoded = event->mimeData()->data( "application/x-qabstractitemmodeldatalist");
    QDataStream stream( &encoded, QIODevice::ReadOnly);

    if ( ui->edit->isReadOnly() )
        goto dropexit;

    if ( !stream.atEnd() ) {
        int row, col;
        QString value;
        QMap < int, QVariant> data;
        stream >> row >> col >> data;
        value = data[0].toString();
        if ( value.right( 5).toUpper() == "[COM]") {
            Communicator comm;
            if ( comm.exec() == QDialog::Accepted )
                value = comm.value();
            else
                goto dropexit;
        }
        if ( (!value.contains( " > ")) || m_editlocked )
             goto dropexit;
        if ( value != ui->edit->text() ) {
            m_oldlock = m_editlocked;
            ui->edit->setText( value);
            ui->btnClear->show();
            m_editlocked = true;
            g_history.fieldModified( this, m_oldvalue, m_oldlock, value, true);
            m_oldvalue = value;
        }
    }
dropexit:
    event->acceptProposedAction();
    m_main->unselectSignal();
}

void Field::on_editingFinished()
{
    if ( m_editlocked ) return;
    if ( m_oldvalue != ui->edit->text() ) {
        if ( m_historyon ) {
            g_history.fieldModified( this, m_oldvalue, m_oldlock, ui->edit->text(), m_editlocked);
        }
        m_oldvalue = ui->edit->text();
        m_oldlock = m_editlocked;
    }
}

void Field::on_textChanged( QString text)
{
    if ( text.isEmpty() ) {
        m_editlocked = false;
        ui->btnClear->hide();
        if ( m_historyon )
            ui->edit->setFocus(); // this forces on_editingFinished to record the changes in g_history
    }

    if ( m_editlocked )
        ui->edit->setText( m_oldvalue);
}

void Field::on_btnClear_clicked()
{
    ui->edit->setText( "");
}
