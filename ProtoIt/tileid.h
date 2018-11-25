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

#ifndef TILEID_H
#define TILEID_H

#include <QDialog>

namespace Ui {
class TileId;
}

class TileId : public QDialog
{
    Q_OBJECT

public:
    explicit TileId(QString device, QWidget *parent = 0);
    ~TileId();

    QString getTileId();

private:
    Ui::TileId *ui;
};

#endif // TILEID_H
