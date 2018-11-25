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

#ifndef RENAMESTEP_H
#define RENAMESTEP_H

#include <QDialog>

namespace Ui {
class RenameStep;
}

class RenameStep : public QDialog
{
    Q_OBJECT

public:
    explicit RenameStep(QWidget *parent = 0);
    ~RenameStep();

    QString name();

private:
    Ui::RenameStep *ui;
};

#endif // RENAMESTEP_H
