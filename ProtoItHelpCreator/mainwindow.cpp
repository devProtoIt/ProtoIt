#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlghelp.h"
#include <QFileDialog>
#include <QTextStream>
#include <QDir>
#include <QShortcut>

enum g_mode { mode_none, mode_platform, mode_author, mode_year, mode_device, mode_copyright, mode_alias, mode_icon, mode_signalin, mode_signalout, mode_signalconst, mode_config};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_ixhelp = -1;
    m_path = QDir::homePath();

    ui->edtScript->setEnabled( false);

    new QShortcut( QKeySequence("Ctrl+Space"), this, SLOT( on_space()));
    new QShortcut( QKeySequence("Ctrl+B"), this, SLOT( on_bold()));
    new QShortcut( QKeySequence("Ctrl+I"), this, SLOT( on_italic()));
    new QShortcut( QKeySequence("Ctrl+U"), this, SLOT( on_underscore()));
    new QShortcut( QKeySequence("Ctrl+Return"), this, SLOT( on_break()));
    new QShortcut( QKeySequence("Ctrl+,"), this, SLOT( on_lessthen()));
    new QShortcut( QKeySequence("Ctrl+."), this, SLOT( on_greaterthen()));
    new QShortcut( QKeySequence("Ctrl+8"), this, SLOT( on_degrees()));
    new QShortcut( QKeySequence("Ctrl+'"), this, SLOT( on_quotes()));

    m_saved = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent( QCloseEvent *event)
{
    if ( !m_saved ) {
        if ( message( QMessageBox::Question, tr( "Er is nog geen help-bestand weggeschreven."), tr( "Toch afsluiten?")) != QMessageBox::Yes ) {
            event->ignore(); // = don't close
            return;
        }
    }
    event->accept(); // = close
}

int MainWindow::message( QMessageBox::Icon icon, QString info, QString ask)
{
    QMessageBox msg;
    msg.setWindowTitle( "ProtoIt");
    msg.setStyleSheet( "background:lightgray");
    msg.setText( info);
    msg.setInformativeText( ask);
    msg.setIcon( icon);
    if ( ask.isEmpty() ) {
        msg.setStandardButtons( QMessageBox::Ok);
        msg.setDefaultButton( QMessageBox::Ok);
    }
    else {
        msg.setStandardButtons( QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton( QMessageBox::No);
        msg.button( QMessageBox::Yes)->setText( tr( "Ja"));
        msg.button( QMessageBox::No)->setText( tr( "Nee"));
    }
    return msg.exec();
}

void MainWindow::on_btnFile_clicked()
{
    if ( !m_saved ) {
        if ( message( QMessageBox::Question, tr( "Er is nog geen help-bestand weggeschreven."), tr( "Toch een nieuwe openen?")) != QMessageBox::Yes ) {
            return;
        }
    }

    g_mode mode = mode_none;
    QFileDialog fd( this, "Open pid-file", QDir::homePath(), "ProtoIt Device (*.pid)");
    fd.setAcceptMode( QFileDialog::AcceptOpen);
    if ( fd.exec() == QDialog::Rejected )
        return;

    ui->edtInfo->clear();
    ui->lstScript->clear();
    ui->edtScript->clear();
    ui->edtScript->setDisabled( true);
    m_help.clear(); // must be called after the former clear() calls.
    m_ixhelp = -1;
    m_platform = "";
    m_author = "";
    m_year = "";
    m_model = "";
    m_icon = "";
    m_path = "";
    m_html = "";

    QString name = fd.selectedFiles().first();
    QFile file( name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in( &file);
    QString line;

    while ( !in.atEnd() ) {
        line = in.readLine().trimmed();
        if ( line.isEmpty() ) continue;

        if ( line.left( 1) == "<" ) {
            mode = mode_none;
            line = line.toUpper();
        }
        else {
            if ( mode == mode_platform || mode == mode_icon || mode == mode_author || mode == mode_year )
                line = QString( "  ") + line;
            else {
                QStringList sl = line.split( ',');
                if ( sl.count() > 1 ) {
                    line = QString( "  ") + sl.at( 1).trimmed();
                    if ( sl.count() > 2 && sl.at( 2).trimmed().left( 2) != "#P" && sl.at( 2).trimmed().left( 2) != "#E" )
                        line += QString( " = ") + sl.at( 2).trimmed();
                }
                else
                    if ( mode != mode_alias)
                        line = "";
            }
        }

        if ( line == "<PLATFORM>" ) mode = mode_platform;
        if ( line == "<AUTHOR>" ) mode = mode_author;
        if ( line == "<YEAR>" ) mode = mode_year;
        if ( line == "<DEVICE>" ) { mode = mode_device; line = QString( "\n") + line; }
        if ( line == "<ALIAS>" ) mode = mode_alias;
        if ( line == "<COPYRIGHT>" ) mode = mode_copyright;
        if ( line == "<ICON>" ) mode = mode_icon;
        if ( line == "<SIGNALIN>" ) mode = mode_signalin;
        if ( line == "<SIGNALOUT>" ) mode = mode_signalout;
        if ( line == "<CONSTANT>" ) mode = mode_signalconst;
        if ( line == "<PORT>" || line == "<CALIBRATE>" ) mode = mode_config;
        if ( mode == mode_none ) continue;

        m_help.append( "");
        ui->lstScript->addItem( line);

        if ( mode == mode_platform )
            m_platform = line.toUpper();

        if ( mode == mode_author )
            m_author = line;

        if ( mode == mode_year )
            m_year = line;

        if ( mode == mode_icon )
            m_icon = line;
    }

    file.close();

    m_model = name.right( name.length() - name.lastIndexOf( '/') - 1);
    m_model = m_model.left( m_model.indexOf( '.'));

    m_path = name.left( name.lastIndexOf( '/')) + "/" + m_model;

    QDir::setCurrent( m_path);
    ui->edtFile->setText( name);

    QUrl base = QUrl::fromLocalFile( QDir::current().absoluteFilePath( "dummy.html"));
    ui->wvwViewer->setHtml( makeHtml(), base);

    m_saved = true;
}

void MainWindow::on_btnModel_clicked()
{
    if ( m_ixhelp >= 0 && m_ixhelp < m_help.count() )
        m_help.replace( m_ixhelp, ui->edtScript->toPlainText());

    QFileDialog fd( this, tr( "Create model template ..."), m_path, "HTML-page (*.html)");
    fd.setAcceptMode( QFileDialog::AcceptSave);
    if ( fd.exec() == QDialog::Rejected )
        return;

    QString name = fd.selectedFiles().first();
    QFile file( name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out( &file);
    out << makeHtml();
    file.close();

    m_saved = true;
}

void MainWindow::on_lstScript_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    int ix = ui->lstScript->currentRow();
    int i;

    for ( i = ix - 1; i > 0; i-- )
        if ( ui->lstScript->item( i)->text().trimmed().left( 1) == "<" )
            break;

    if ( i < 0 ) return;

    if ( ui->lstScript->item( i)->text().trimmed().toUpper() != "<SIGNALIN>" &&
         ui->lstScript->item( i)->text().trimmed().toUpper() != "<SIGNALOUT>" &&
         ui->lstScript->item( i)->text().trimmed().toUpper() != "<PORT>" &&
         ui->lstScript->item( i)->text().trimmed().toUpper() != "<CALIBRATE>" ) {
        m_ixhelp = -1;
        ui->edtScript->setPlainText( "");
        ui->edtScript->setDisabled( true);
        return;
    }

    ui->edtScript->setEnabled( true);

    if ( ix >= 0 && ix < m_help.count() ) {
        m_ixhelp = ix;
        ui->edtScript->setPlainText( m_help.at( m_ixhelp));
    }
}

void MainWindow::on_btnHelp1_clicked()
{
    DlgHelp dlg;
    dlg.exec();
}

void MainWindow::on_btnHelp2_clicked()
{
    DlgHelp dlg;
    dlg.exec();
}

void MainWindow::on_edtInfo_textChanged()
{
    m_saved = false;
    QUrl base = QUrl::fromLocalFile( QDir::current().absoluteFilePath( "dummy.html"));
    ui->wvwViewer->setHtml( makeHtml(), base);
}

void MainWindow::on_edtScript_textChanged()
{
    m_saved = false;
    if ( m_ixhelp >= 0 && m_ixhelp < m_help.count() )
        m_help.replace( m_ixhelp, ui->edtScript->toPlainText());
    QUrl base = QUrl::fromLocalFile( QDir::current().absoluteFilePath( "dummy.html"));
    ui->wvwViewer->setHtml( makeHtml(), base);
    if ( m_ixhelp >= 0 && m_ixhelp < ui->lstScript->count() )
        ui->wvwViewer->findText( ui->lstScript->item( m_ixhelp)->text().trimmed());
}

void MainWindow::on_space()
{
    QStringList tags;
    int i;
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        QString str = qte->toPlainText().left( qtc.position());
        i = 0;
        while ( i < str.length() - 2 ) {
            if ( str.at( i) == '<' ) {
                i++;
                if ( str.at( i) == '/' ) {
                    i++;
                    for ( int j = tags.count() - 1; j >= 0; j-- )
                        if ( tags.at( j) == str.at( i) ) {
                            tags.removeAt( j);
                            break;
                        }
                }
                else
                    tags.append( str.at( i));
                i++; // skip '>'
            }
            i++;
        }
        if ( tags.count() )
            qtc.insertText( QString( "</") + tags.at( tags.count() - 1) + ">");
    }
}

void MainWindow::on_bold()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "<b>");
    }
}

void MainWindow::on_italic()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "<i>");
    }
}

void MainWindow::on_underscore()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "<u>");
    }
}

void MainWindow::on_break()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "<br/>");
    }
}

void MainWindow::on_lessthen()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "&lt;");
    }
}

void MainWindow::on_greaterthen()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "&gt;");
    }
}

void MainWindow::on_degrees()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "&deg;");
    }
}

void MainWindow::on_quotes()
{
    QPlainTextEdit *qte = (QPlainTextEdit*) focusWidget();
    if ( qte == ui->edtInfo || qte == ui->edtScript ) {
        QTextCursor qtc = qte->textCursor();
        qtc.insertText( "&quot;");
    }
}

QString MainWindow::makeHtml()
{
    QString out, line;
    QString devname, devdesc;
    int ix;
    bool hasconfig = false;

    out += "<html>\n";
    out += "<head>\n";
    out += "<title> ProtoIt </title>\n";
    out += "<style type=""text/css"">\n";
    out += "body { background-color:#D7EAF9; font-family:Century Gothic; font-size:14px; font-weight:400; }\n";
    out += "table { font-family:Century Gothic; font-size:14px; font-weight:400; }\n";
    out += "td { vertical-align:top; }\n";
    out += "td.left { width:150; font-family:Century Gothic; font-size:14px; font-weight:400; }\n";
    out += "td.right { width:350; font-family:Century Gothic; font-size:14px; font-weight:400; }\n";
    out += "h1 { color:#FF7F2F; }\n";
    out += "h4 { color:#FF7F2F; }\n";
    out += "</style>\n";
    out += "</head>\n\n";
    out += "<body>\n\n";

    out += "<table>\n";
    out += "<tr><td width=400><h1>" + m_model + "</h1><br/><br/>";
    if ( !m_author.isEmpty() && !m_year.isEmpty() )
        out += "&copy; " + m_year + ", " + m_author + "<br/><br/>\n";
    out += "<b>Platform: " + m_platform + "</b></td>";
    out += "<td><img src=\"" + m_model + ".png\" height=100 /></td></tr>\n";
    out += "</table>\n<hr/>\n\n";

    out += "<p><u>Beschrijving:</u><br/>\n" + ui->edtInfo->toPlainText() + "\n</p>\n\n";
    out += "<p><u>Onderdelen:</u><br/>\n";

    QString sep;
    QString dev;
    g_mode mode = mode_none;
    for ( ix = 0; ix < ui->lstScript->count(); ix++ ) {
        line = ui->lstScript->item( ix)->text().trimmed();
        if ( line.isEmpty() ) continue;
        if ( line.left( 1) == "<" ) {
            if ( line == "<DEVICE>" ) {
                mode = mode_device;
                if ( !dev.isEmpty() ) {
                    out += sep + dev;
                    sep = ", ";
                }
            }
            if ( line == "<ALIAS>" )
                mode = mode_alias;
            continue;
        }
        if ( mode != mode_none ) // this means that first dev becomes the device name and
            dev = line;          // if an alias exists the device name becomes overwritten by the alias
        mode = mode_none;
    }
    if ( !dev.isEmpty() )
        out += sep + dev;

    out += "\n</p>\n<hr/>\n";

    mode = mode_none;
    for ( ix = 0; ix < ui->lstScript->count(); ix++ ) {
        line = ui->lstScript->item( ix)->text().trimmed();
        if ( line.isEmpty() ) continue;

        if ( line.left( 1) == "<" &&
              (mode == mode_signalin || mode == mode_signalout || mode == mode_config) )
            out += "</table><br/>\n";

        if ( line == "<DEVICE>" ) {
            mode = mode_device;
            hasconfig = false;
        }
        if ( line == "<ICON>" ) mode = mode_icon;
        if ( line == "<CONSTANT>" ) mode = mode_signalconst;

        if ( line == "<PORT>" || line == "<CALIBRATE>" ) {
            mode = mode_config;
            if ( !hasconfig ) {
                out += "\n<u>Instellingen:</u>\n";
                hasconfig = true;
            }
            out += "<table>\n";
        }
        if ( line == "<SIGNALIN>" ) {
            mode = mode_signalin;
            out += "\n<u>Op de programmategel:</u>\n<table>\n";
        }
        if ( line == "<SIGNALOUT>" ) {
            mode = mode_signalout;
            out += "\n<u>Zendt de signalen:</u>\n<table>\n";
        }

        if ( line.left( 1) == "<" ) continue;
        if ( mode == mode_none ) continue;

        if ( mode == mode_device ) {
            devname = line;
            devdesc = m_help.at( ix);
            continue;
        }

        if ( mode == mode_icon ) {
            out += "\n<h4><img src=\"" + line + "\" width=50> " + devname + "</h4>\n";
            out += "<p>\n" + devdesc + "\n</p>\n";
        }

        if ( mode == mode_signalin || mode == mode_signalout || mode == mode_config ) {
            int x = line.lastIndexOf( " = ");
            if ( x >= 0 ) line = line.left( x);
            out += "<tr><td class=\"left\"><b>" + line + "</b></td><td>" + m_help.at( ix) + "</td></tr>\n";
        }
    }
    out += "</table><br/>\n\n";

    out += "<br/>\n</body>\n</html>\n";

    return out;
}
