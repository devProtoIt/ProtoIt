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

#include "history.h"
#include "mainwindow.h"
#include "step.h"
#include "tile.h"
#include "historyreport.h"
#include <QDialog>

#define HIST_APPEND 0
#define HIST_UNDO   1
#define HIST_REDO   2

History g_history;

History::History()
{
    m_main = NULL;
    m_ixhr = -1;
    m_saved = true;
}

History::~History()
{
}

void History::setMain( MainWindow* main)
{
    m_main = main;
}

HistoryRecord* History::add( HistoryObject ho, HistoryAction ha)
{
    removeTail();
    HistoryRecord* hr = new HistoryRecord;
    hr->object = ho;
    hr->action = ha;
    hr->templ = NULL;
    hr->step = NULL;
    hr->tile = NULL;
    hr->field = NULL;
    hr->oldix = hr->newix = -1;
    m_hr.append( hr);
    m_ixhr = m_hr.count() - 1;
    m_saved = false;
    return hr;
}

void History::deviceCreated( Template* templ, int index)
{
    HistoryRecord *hr = add( hoTemplate, haCreate);
    hr->templ = templ;
    hr->newix = index;
    m_report.append( makeReport( HIST_APPEND));
}

void History::stepCreated( Step* step, int index, QString name)
{
    HistoryRecord *hr = add( hoStep, haCreate);
    hr->step = step;
    hr->newix = index;
    hr->newvalue = name;
    m_report.append( makeReport( HIST_APPEND));
}

void History::tileCreated( Tile* tile, int index)
{
    HistoryRecord *hr = add( hoTile, haCreate);
    hr->tile = tile;
    hr->newix = index;
    m_report.append( makeReport( HIST_APPEND));
}

void History::deviceDeleted( Template* templ, int index)
{
    HistoryRecord *hr = add( hoTemplate, haDelete);
    hr->templ = templ;
    hr->oldix = index;
    m_report.append( makeReport( HIST_APPEND));
}

void History::stepDeleted( Step* step, int index, QString name)
{
    HistoryRecord *hr = add( hoStep, haDelete);
    hr->step = step;
    hr->oldix = index;
    hr->oldvalue = name;
    m_report.append( makeReport( HIST_APPEND));
}

void History::tileDeleted( Tile* tile, int index)
{
    HistoryRecord *hr = add( hoTile, haDelete);
    hr->tile = tile;
    hr->oldix = index;
    m_report.append( makeReport( HIST_APPEND));
}

void History::stepMoved( int oldix, int newix)
{
    HistoryRecord *hr = add( hoStep, haMove);
    hr->oldix = oldix;
    hr->newix = newix;
    m_report.append( makeReport( HIST_APPEND));
}

void History::stepRenamed( int index, QString oldname, QString newname)
{
    HistoryRecord *hr = add( hoStep, haUpdate);
    hr->oldix = hr->newix = index;
    hr->oldvalue = oldname;
    hr->newvalue = newname;
    m_report.append( makeReport( HIST_APPEND));
}

void History::fieldModified( Field* field, QString oldvalue, bool oldlock, QString newvalue, bool newlock)
{
    HistoryRecord *hr = add( hoField, haUpdate);
    hr->field = field;
    hr->oldvalue = oldvalue;
    hr->newvalue = newvalue;
    m_report.append( makeReport( HIST_APPEND));
}

void History::undo()
{
    if ( m_ixhr < 0 ) return;
    m_saved = false;
    //m_main->message( QMessageBox::Information, report());

    HistoryRecord* hr = m_hr.at( m_ixhr);
    switch ( hr->action ) {
    case haCreate : switch ( hr->object ) {
                    case hoTemplate : unCreateDevice( hr); break;
                    case hoStep : unCreateStep( hr); break;
                    case hoTile : unCreateTile( hr); break;
                    } break;
    case haDelete : switch ( hr->object ) {
                    case hoTemplate : unDeleteDevice( hr); break;
                    case hoStep : unDeleteStep( hr); break;
                    case hoTile : unDeleteTile( hr); break;
                    } break;
    case haUpdate : switch ( hr->object ) {
                    case hoStep  : unRenameStep( hr); break;
                    case hoField : unModifyField( hr); break;
                    } break;
    case haMove :   unMoveStep( hr); break;
    }

    m_ixhr--;
}

void History::redo()
{
    if ( m_ixhr >= m_hr.count() - 1 )return;
    m_saved = false;
    //m_main->message( QMessageBox::Information, report());

    m_ixhr++;

    HistoryRecord* hr = m_hr.at( m_ixhr);
    switch ( hr->action ) {
    case haCreate : switch ( hr->object ) {
                    case hoTemplate : reCreateDevice( hr); break;
                    case hoStep : reCreateStep( hr); break;
                    case hoTile : reCreateTile( hr); break;
                    } break;
    case haDelete : switch ( hr->object ) {
                    case hoTemplate : reDeleteDevice( hr); break;
                    case hoStep : reDeleteStep( hr); break;
                    case hoTile : reDeleteTile( hr); break;
                    } break;
    case haUpdate : switch ( hr->object ) {
                    case hoStep  : reRenameStep( hr); break;
                    case hoField : reModifyField( hr); break;
                    } break;
    case haMove :   reMoveStep( hr); break;
    }
}

void History::unCreateDevice( HistoryRecord* hr)
{
    m_main->removeDevice( hr->templ->alias());
    m_main->getTab()->setCurrentIndex( 0);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unCreateStep( HistoryRecord* hr)
{
    m_main->removeStep( hr->newix);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unCreateTile( HistoryRecord* hr)
{
    Step* step = hr->tile->step();
    step->removeTile( hr->tile->templ()->alias());
    m_main->getTab()->setCurrentWidget( step);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unDeleteDevice( HistoryRecord* hr)
{
    m_main->insertDevice( hr->templ, hr->oldix);
    m_main->getTab()->setCurrentIndex( 0);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unDeleteStep( HistoryRecord* hr)
{
    m_main->insertStep( hr->step, hr->oldix, hr->oldvalue);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unDeleteTile( HistoryRecord* hr)
{
    Step* step = hr->tile->step();
    step->insertTile( hr->tile, hr->oldix);
    m_main->getTab()->setCurrentWidget( step);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unMoveStep( HistoryRecord* hr)
{
    m_main->moveStep( hr->newix, hr->oldix);
    m_main->getTab()->setCurrentIndex( hr->oldix);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unRenameStep( HistoryRecord* hr)
{
    m_main->getTab()->setTabText( hr->oldix, hr->oldvalue);
    m_main->getTab()->setCurrentIndex( hr->oldix);
    m_report.append( makeReport( HIST_UNDO));
}

void History::unModifyField( HistoryRecord* hr)
{
    hr->field->setValue( hr->oldvalue, true, false);
    hr->field->tile()->setFocus();
    m_main->getTab()->setCurrentWidget( hr->field->tile()->step());
    m_report.append( makeReport( HIST_UNDO));
}

void History::reCreateDevice( HistoryRecord* hr)
{
    m_main->insertDevice( hr->templ, hr->newix);
    m_main->getTab()->setCurrentIndex( 0);
    m_report.append( makeReport( HIST_REDO));
}

void History::reCreateStep( HistoryRecord* hr)
{
    m_main->insertStep( hr->step, hr->newix, hr->newvalue);
    m_report.append( makeReport( HIST_REDO));
}

void History::reCreateTile( HistoryRecord* hr)
{
    Step* step = hr->tile->step();
    step->insertTile( hr->tile, hr->oldix);
    m_main->getTab()->setCurrentWidget( step);
    m_report.append( makeReport( HIST_REDO));
}

void History::reDeleteDevice( HistoryRecord* hr)
{
    m_main->removeDevice( hr->templ->alias());
    m_main->getTab()->setCurrentIndex( 0);
    m_report.append( makeReport( HIST_REDO));
}

void History::reDeleteStep( HistoryRecord* hr)
{
    m_main->removeStep( hr->newix);
    m_report.append( makeReport( HIST_REDO));
}

void History::reDeleteTile( HistoryRecord* hr)
{
    Step* step = hr->tile->step();
    step->removeTile( hr->tile->templ()->alias());
    m_main->getTab()->setCurrentWidget( step);
    m_report.append( makeReport( HIST_REDO));
}

void History::reMoveStep( HistoryRecord* hr)
{
    m_main->moveStep( hr->oldix, hr->newix);
    m_main->getTab()->setCurrentIndex( hr->newix);
    m_report.append( makeReport( HIST_REDO));
}

void History::reRenameStep( HistoryRecord* hr)
{
    m_main->getTab()->setTabText( hr->newix, hr->newvalue);
    m_main->getTab()->setCurrentIndex( hr->newix);
    m_report.append( makeReport( HIST_REDO));
}

void History::reModifyField( HistoryRecord* hr)
{
    hr->field->setValue( hr->newvalue, true, false);
    hr->field->tile()->setFocus();
    m_main->getTab()->setCurrentWidget( hr->field->tile()->step());
    m_report.append( makeReport( HIST_REDO));
}

void History::setSaved( bool state)
{
    m_saved = state;
}

bool History::isSaved()
{
    return m_saved;
}

void History::removeAll()
{
    // all templates, tiles and steps of the tail must be deleted
    removeTail();
    // those still in use are deleted by the parent class
    // simply remove them from the history
    for ( int i = 0; i < m_hr.count(); i++ )
        delete m_hr.at( i);
    m_hr.clear();
    m_ixhr = -1;
    m_report.clear();
}

void History::removeTail()
{
    HistoryRecord* hr;
    for ( int i = m_hr.count() - 1; i > m_ixhr; i-- ) {
        hr = m_hr.at( i);
        if ( hr->object == hoTile && hr->action == haCreate )
            delete hr->tile;
        if ( hr->object == hoStep && hr->action == haCreate )
            delete hr->step;
        if ( hr->object == hoTemplate && hr->action == haCreate )
            delete hr->templ;
        delete hr;
        m_hr.removeAt( i);
    }
}

QString History::makeReport( int type)
{
    if ( m_ixhr < 0 || m_ixhr >= m_hr.count() ) return tr( "FOUT:     Index bestaat niet.");

    QString line;

    switch ( type ) {
    case HIST_UNDO :    line = tr( "Ongedaan gemaakt: "); break;
    case HIST_REDO :    line = tr( "Weer hersteld: "); break;
    }

    HistoryRecord* hr = m_hr[ m_ixhr];
    switch ( hr->action ) {
        case haCreate : switch ( hr->object ) {
                            case hoTemplate : line += tr( "Apparaat '") + hr->templ->alias() + tr( "' aangemaakt."); break;
                            case hoTile :     line += tr( "Tegel '") + hr->tile->alias() + tr( "' aangemaakt."); break;
                            case hoStep :     line += tr( "Stap ") + QString::number( hr->step->index()) + tr( " aangemaakt."); break;
                            }
                        break;
        case haMove : line += tr( "Stap ") + QString::number( hr->oldix) + tr( " verplaatst naar ") + QString::number( hr->newix) + ".";
                      break;
        case haUpdate : switch ( hr->object ) {
                            case hoStep : line += tr( "Stap ") + QString::number( hr->oldix) + tr( " veranderd van '");  break;
                            case hoField : line += tr( "'") + hr->field->tile()->alias() + ":" + hr->field->alias() + "' veranderd van '"; break;
                        }
                        line += hr->oldvalue + tr( "' in '") + hr->newvalue + "'.";
                        break;
        case haDelete : switch ( hr->object ) {
                            case hoTemplate : line += tr( "Apparaat '") + hr->templ->alias() + tr( "' verwijderd."); break;
                            case hoTile :     line += tr( "Tegel '") + hr->tile->alias() + tr( "' verwijderd."); break;
                            case hoStep :     line += tr( "Stap ") + QString::number( hr->step->index()) + tr( " verwijderd."); break;
                            }
                        break;
    }

    return line;
}

void History::addReport( QString msg)
{
    m_report.append( msg);
}

void History::report()
{
    HistoryReport hr;
    for ( int i = 0; i < m_report.count(); i++ )
        hr.addRow( m_report.at( i));
    hr.exec();
}
