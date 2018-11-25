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

#include "tileid.h"
#include "ui_tileid.h"
#include <QPushButton>

TileId::TileId(QString device, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TileId)
{
    ui->setupUi(this);
    ui->editID->setFocus();
    ui->editDev->setText( device);

    ui->buttonBox->button( QDialogButtonBox::Ok)->setText( tr( "Voeg toe"));
    ui->buttonBox->button( QDialogButtonBox::Cancel)->setText( tr( "Annuleer"));
}

TileId::~TileId()
{
    delete ui;
}

QString TileId::getTileId()
{
    return ui->editID->text().trimmed();
}
