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

#include "info.h"
#include "ui_info.h"

Info::Info(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Info)
{
    ui->setupUi(this);
    QString str =
            QString( "ProtoIt is ontstaan vanuit de behoefte om in een educatieve setting robots te programmeren. ") +
            QString( "De werkwijze sluit aan bij de intuitieve aanpak van leerlingen om een programma in stappen te ontwikkelen. ") +
            QString( "De leerlingen schrijven geen programmacode, maar in iedere stap definiëren ze regels voor het functioneren van de robot. ") +
            QString( "Gaandeweg bouwen ze aan een programma, zonder gefrustreerd te raken door verkeerd geplaatste haakjes of ") +
            QString( "een ingewikkelde program flow.");
    ui->label->setText( str);
}

Info::~Info()
{
    delete ui;
}
