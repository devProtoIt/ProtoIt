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

#ifndef TILE_H
#define TILE_H

#include <QDialog>
#include "template.h"
#include "field.h"
#include "codetextedit.h"

extern const char* SENSOR_COLOR;
extern const char* ACTUATOR_COLOR;
extern const char* FUNCTION_COLOR;

class MainWindow;
class Step;

namespace Ui {
class Tile;
}

class Tile : public QDialog
{
    Q_OBJECT

public:
    explicit Tile( Step *parent = 0, Template* templ = 0, MainWindow *main = 0, bool hasexpand = false);
    ~Tile();

    void setDevice( DeviceType dt, QString device, QString alias, QString icon);
    void setStep( Step* step);

    Template*    templ();
    Step*        step();
    QString      alias();

    void createHeader( QString text, bool subhead = false);
    void createField( QString signal, QString alias, QString init, FieldType ft, bool vtab);
    void buildTile();
    void flushField();
    void initField( QString alias, QString value, int fieldix = 0);
    Field* field( QString alias);
    Field* firstField();
    Field* nextField();

    QString findSignalKey( QString signal);
    QString findKeySignal( QString key);

    void setPos( int x, int y);
    void scrollTo( int scroll);

    bool isDeviceUsed( QString alias);      // device alias
                                            // checks if one of the device's signals is used
private slots:

    void on_butDelete_clicked();
    void on_butExpand_clicked();
    void on_butCode_clicked();
    void on_edtCode_textChanged();
    void on_edtCode_selectionChanged();
    void on_butUnlock_clicked();

protected:

    void keyPressEvent( QKeyEvent * e);

private:

    void resetCode();
    void setCode( QString code, int cursorpos = -1);

    Ui::Tile *ui;

    MainWindow*     m_main;
    CodeTextEdit*   m_code;
    QString         m_oldcode;
    int             m_oldpos;
    Template*       m_templ;
    Step*           m_step;
    QList<Field*>   m_fields;
    int             m_ix;                   // index to the current field

    int             m_xpos;                 // position of the tile
    int             m_ypos;
    bool            m_expanded;
    bool            m_codemode;
    bool            m_codechanged;
    bool            m_blockselection;

    int             m_xctrl;                // position for the next field to create
    int             m_yctrl;
};

#endif // TILE_H
