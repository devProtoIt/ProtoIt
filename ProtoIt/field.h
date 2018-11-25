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

#ifndef FIELD_H
#define FIELD_H

#include <QWidget>

class MainWindow;
class Tile;

enum FieldType { ftField, ftLabel, ftConstant, ftExpanded, ftHeader, ftSubhead };

namespace Ui {
class Field;
}

class Field : public QWidget
{
    Q_OBJECT

public:
    explicit Field( MainWindow* main, Tile *tile, QString key, QString alias, int xpos, int ypos, FieldType ft, bool vtab);
    ~Field();

    FieldType type();
    QString key();
    QString alias();
    QString value();
    bool    isLocked();
    bool vTab();
    void initValue( QString value);
    void setValue( QString value, bool unlock = false, bool historyon = true);
    void setReadonly( bool readonly);
    Tile* tile();

protected:

    void dragEnterEvent( QDragEnterEvent *event);
    void dropEvent( QDropEvent* event);

private slots:

    void on_editingFinished();
    void on_textChanged( QString text);
    void on_btnClear_clicked();

private:
    Ui::Field * ui;
    MainWindow* m_main;
    Tile*       m_tile;
    QString     m_key;
    QString     m_oldvalue;
    bool        m_editlocked; // editing is locked after dragging a signal
                              // editing is freed again after deleting a signal
    bool        m_historyon;  // editing must result in a historyrecord
    bool        m_oldlock;
    bool        m_vtab;       // this field is preceeded by a vertical tab
    bool        m_extended;   // this field is shown in extended mode only
    FieldType   m_ft;
};

#endif // FIELD_H
