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

#include "renamestep.h"
#include "ui_renamestep.h"
#include <QPushButton>

RenameStep::RenameStep(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameStep)
{
    ui->setupUi(this);
    ui->edtName->setFocus();

    ui->buttonBox->button( QDialogButtonBox::Cancel)->setText( tr( "Annuleer"));
}

RenameStep::~RenameStep()
{
    delete ui;
}

QString RenameStep::name()
{
    return ui->edtName->text().trimmed();
}
