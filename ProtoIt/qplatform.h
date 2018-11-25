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

#ifndef QPLATFORM_H
#define QPLATFORM_H

#include <QDialog>
#include <QRadioButton>

namespace Ui {
class QPlatform;
}

class QPlatform : public QDialog
{
    Q_OBJECT

public:
    explicit QPlatform(QWidget *parent = 0);
    ~QPlatform();

    void setPlatforms( QString path, QStringList& platforms);
    int selectedPlatform();

private:
    Ui::QPlatform *ui;

    QList<QRadioButton*>    m_rbut;
    QString                 m_path;
    QStringList             m_platforms;
    int                     m_current;
};

#endif // QPLATFORM_H
