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

#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QDialog>
#include <QListWidgetItem>
#include <QMediaPlayer>
#include <QVideoWidget>

namespace Ui {
class HelpViewer;
}

class HelpViewer : public QDialog
{
    Q_OBJECT

public:
    explicit HelpViewer(QWidget *parent = 0);
    ~HelpViewer();

    void setHelp( QString filepath);

private slots:
    void on_lstHelp_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_btnSearch_clicked();
    void on_btnClear_clicked();

private:
    Ui::HelpViewer *ui;

    QStringList m_help;

    QString     m_page;
};

#endif // HELPVIEWER_H
