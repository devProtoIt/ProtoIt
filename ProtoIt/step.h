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

#ifndef STEP_H
#define STEP_H

#include <QWidget>

class MainWindow;
class Tile;
class Device;

class Step : public QWidget
{
    Q_OBJECT

public:
    explicit Step(MainWindow *parent = 0);

    void setScroll( int scroll);
    int  getScroll();
    int  sizeY();

    void setIndex( int ix);
    int  index();
    bool isConfig();

    void modelAllTiles();
    void modelTile( int tilewidth, int tileheight, int &xpos, int &ypos);

    void appendTile( Tile* tile);
    void insertTile( Tile* tile, int ix);
    void removeTile( QString alias);
    void deleteAllTiles();

    Tile*   firstTile();
    Tile*   nextTile();
    Tile*   tile( QString alias);         // device alias
    int     ixTile( QString alias);       // device alias

private:

    MainWindow*     m_main;
    QList<Tile*>    m_tiles;
    int             m_ixtile;

    int m_index;
    int m_scroll;
    int m_xpos;
    int m_ypos;
    int m_width;
    int m_height;
};

#endif // STEP_H
