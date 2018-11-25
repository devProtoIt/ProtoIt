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

#include "qplatform.h"
#include "ui_qplatform.h"
#include "mainwindow.h"
#include <QIcon>

QPlatform::QPlatform(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QPlatform)
{
    ui->setupUi(this);
}

QPlatform::~QPlatform()
{
    delete ui;
}

void QPlatform::setPlatforms( QString path, QStringList& platforms)
{
    if ( platforms.isEmpty() ) {
        ui->lblChoose->hide();
        ui->lblBrands->setText( tr( "ProtoIt WERD ZONDER PLATFORM GEÏNSTALLEERD - installeer opnieuw"));
        return;
    }

    QRadioButton* rbut;
    QIcon* icon;
    int dy = platforms.count() * 22;

    resize( width(), height() + dy);
    ui->buttonBox->move( ui->buttonBox->x(), ui->buttonBox->y() + dy);
    ui->lineBottom->move( ui->lineBottom->x(), ui->lineBottom->y() + dy);
    ui->lblBrands->move( ui->lblBrands->x(), ui->lblBrands->y() + dy);

    for ( int i = 0; i < platforms.count(); i++ ) {
        icon = new QIcon( path + platforms.at( i) + "/" + platforms.at( i) + ".png");
        rbut = new QRadioButton( this);
        m_rbut.append( rbut);
        rbut->setIcon( *icon);
        rbut->setIconSize( icon->actualSize( QSize( 100, 20)));
        rbut->move( 100, 110 + i * 22);
        rbut->resize( 150, 20);
    }
    m_rbut.at( 0)->setChecked( true);
}

int QPlatform::selectedPlatform()
{
    for ( int i = 0; i < m_rbut.count(); i++ )
        if ( m_rbut.at( i)->isChecked() )
            return i;
    return -1;
}
