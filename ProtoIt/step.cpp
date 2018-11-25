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

#include "step.h"
#include "mainwindow.h"

Step::Step(MainWindow *parent) :
    QWidget(parent)
{
    m_xpos = 0;
    m_ypos = 10;
    m_width = 0;
    m_height = 0;
    m_scroll = 0;
    m_main = parent;
    m_ixtile = 0;
}

void Step::setIndex( int ix)
{
    m_index = ix;
}

int Step::index()
{
    return m_index;
}

bool Step::isConfig()
{
    return (m_index == 0);
}

void Step::setScroll( int scroll)
{
    m_scroll = scroll;
}

int Step::getScroll()
{
    return m_scroll;
}

int Step::sizeY()
{
    return m_ypos + m_height + 20;
}

void Step::modelTile( int tilewidth, int tileheight, int &xpos, int &ypos)
{
    m_xpos += m_width + 10;
    m_width = tilewidth;

    if ( m_xpos + m_width  > width() )
    {
        m_xpos = 10;
        m_ypos += m_height + 10;
        m_height = tileheight;
    }
    else if ( tileheight > m_height )
        m_height = tileheight;

    xpos = m_xpos;
    ypos = m_ypos;
}

void Step::modelAllTiles()
{
    m_xpos = 0;
    m_ypos = 10;
    m_width = 0;
    m_height = 0;
    m_scroll = 0;

    int x, y;
    Tile* tile;

    for ( int i = 0; i < m_tiles.count(); i++ ) {
        tile = m_tiles.at( i);
        modelTile( tile->width(), tile->height(), x, y);
        tile->setPos( x, y);
        tile->move( x, y);
    }

    m_main->adjustScroll();
}

void Step::removeTile( QString alias)
{
    int i;
    for ( i = 0; i < m_tiles.count(); i++ )
        if ( m_tiles.at( i)->alias() == alias ) break;
    if ( i >= m_tiles.count() ) return;
    Tile* tile = m_tiles.at( i);
    m_tiles.removeAt( i); // actual deletion is done by history class
    tile->setParent( m_main->addStep());
    modelAllTiles();
}

void Step::deleteAllTiles()
{
    for ( int i = m_tiles.count() - 1; i >= 0; i-- )
        delete m_tiles.at( i);
    m_tiles.clear();
}

void Step::appendTile( Tile* tile)
{
    tile->setParent( this);
    m_tiles.append( tile);
    tile->show();
    modelAllTiles();
}

void Step::insertTile( Tile* tile, int ix)
{
    tile->setParent( this);
    m_tiles.insert( ix, tile);
    tile->show();
    modelAllTiles();
}

Tile* Step::firstTile()
{
    m_ixtile = 0;
    if ( m_tiles.count() )
        return m_tiles.at( 0);
    return NULL;
}

Tile* Step::nextTile()
{
    if ( m_ixtile + 1 < m_tiles.count() ) {
        m_ixtile++;
        return m_tiles.at( m_ixtile);
    }
    return NULL;
}

int Step::ixTile( QString alias)
{
    int i;
    for ( i = 0; i < m_tiles.count(); i++ )
        if ( m_tiles.at( i)->alias() == alias )
            return i;
    return -1;
}

Tile* Step::tile( QString alias)
{
    int i;
    for ( i = 0; i < m_tiles.count(); i++ )
        if ( m_tiles.at( i)->alias() == alias )
            return m_tiles.at( i);
    return NULL;
}
