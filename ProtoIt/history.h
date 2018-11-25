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

#ifndef HISTORY_H
#define HISTORY_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QCoreApplication>

class MainWindow;
class Step;
class Template;
class Tile;
class Field;

enum HistoryObject { hoTemplate, hoStep, hoTile, hoField };
enum HistoryAction { haCreate, haMove, haUpdate, haDelete };

typedef struct historyrecordstruct
{
    HistoryObject   object;
    HistoryAction   action;

    Template*       templ;
    Tile*           tile;
    Field*          field;
    Step*           step;

    int             oldix;
    int             newix;
    QString         oldvalue;
    QString         newvalue;
} HistoryRecord;

class History
{
    Q_DECLARE_TR_FUNCTIONS(History)

public:
    History();
    ~History();

    void setMain( MainWindow* main);

    void deviceCreated( Template* templ, int index);
    void stepCreated( Step* step, int index, QString name);
    void tileCreated( Tile* tile, int index);

    void deviceDeleted( Template* templ, int index);
    void stepDeleted( Step* step, int index, QString name);
    void tileDeleted( Tile* tile, int index);

    void stepMoved( int oldix, int newix);
    void stepRenamed( int index, QString oldname, QString newname);
    void fieldModified( Field* field, QString oldvalue, bool oldlock, QString newvalue, bool newlock);

    void undo();
    void redo();

    void removeAll();
    void removeTail();

    void setSaved( bool state = true);
    bool isSaved();

    void addReport( QString msg);
    void report();

protected:

    HistoryRecord* add( HistoryObject ho, HistoryAction ha);

    void unCreateDevice( HistoryRecord* hr);
    void unCreateStep( HistoryRecord* hr);
    void unCreateTile( HistoryRecord* hr);

    void unDeleteDevice( HistoryRecord* hr);
    void unDeleteStep( HistoryRecord* hr);
    void unDeleteTile( HistoryRecord* hr);

    void unMoveStep( HistoryRecord* hr);
    void unRenameStep( HistoryRecord* hr);
    void unModifyField( HistoryRecord* hr);

    void reCreateDevice( HistoryRecord* hr);
    void reCreateStep( HistoryRecord* hr);
    void reCreateTile( HistoryRecord* hr);

    void reDeleteDevice( HistoryRecord* hr);
    void reDeleteStep( HistoryRecord* hr);
    void reDeleteTile( HistoryRecord* hr);

    void reMoveStep( HistoryRecord* hr);
    void reRenameStep( HistoryRecord* hr);
    void reModifyField( HistoryRecord* hr);

    void removeTail( HistoryRecord* hr);
    QString makeReport( int type);

    MainWindow*             m_main;
    QList<HistoryRecord*>   m_hr;
    int                     m_ixhr;
    bool                    m_saved;
    QStringList             m_report;
};

extern History g_history;

#endif // HISTORY_H
