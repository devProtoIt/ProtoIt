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

#include <QWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QPixmap>
#include <QScrollBar>
#include "tile.h"
#include "ui_tile.h"
#include "mainwindow.h"

const char* SENSOR_COLOR = "background:rgb(224,255,224);";
const char* ACTUATOR_COLOR = "background:lightyellow;";
const char* FUNCTION_COLOR = "background:rgb(240,240,240);";

Tile::Tile( Step *parent, Template* templ, MainWindow *main, bool hasexpand) :
    QDialog(parent),
    ui(new Ui::Tile)
{
    ui->setupUi(this);
    setWindowFlags( Qt::FramelessWindowHint);
    m_main = main;
    m_templ = templ;
    m_step = parent;
    m_xctrl = 10;
    m_yctrl = 60;
    m_expanded = false;
    m_codemode = false;
    m_codechanged = false;
    m_blockselection = false;
    m_code = new CodeTextEdit( this);
    m_code->move( 10, 44);
    m_code->setMouseTracking( true);
    m_code->setCenterOnScroll( true);
    connect( m_code, SIGNAL(textChanged()), this, SLOT(on_edtCode_textChanged()));
    connect( m_code, SIGNAL(selectionChanged()), this, SLOT(on_edtCode_selectionChanged()));
    if ( !hasexpand )
        ui->butExpand->hide();
    if ( !m_codechanged )
        resetCode();
    m_code->hide();
}

Tile::~Tile()
{
    for ( int i = 0; i < m_fields.count(); i++ )
        delete m_fields.at( i);
    m_fields.clear();
    delete ui;
}

void Tile::keyPressEvent( QKeyEvent * e)
{
    if ( e->key() == Qt::Key_Escape ) return;       // block usage of escape-key
    if ( e->key() == Qt::Key_Control ) setFocus();  // make ctrl-Z and ctrl-Y work on the tile itself and not on a child
}

void Tile::on_butDelete_clicked()
{
    if ( m_step->isConfig() && m_main->isDeviceUsed( alias()) )
        return;
    // create history record
    if ( m_step->isConfig() ) {
        g_history.deviceDeleted( m_templ, m_step->ixTile( m_templ->alias()));
        m_main->removeDevice( m_templ->alias());
    }
    else {
        g_history.tileDeleted( this, m_step->ixTile( m_templ->alias()));
        m_step->removeTile( m_templ->alias());
    }
}

void Tile::setDevice( DeviceType dt, QString device, QString alias, QString icon)
{
    switch ( dt ) {
    case dtActuator : setStyleSheet( ACTUATOR_COLOR); break;
    case dtSensor : setStyleSheet( SENSOR_COLOR); break;
    case dtFunction : setStyleSheet( FUNCTION_COLOR); break;
    }

    ui->lblDevice->setText( device);
    ui->lblAlias->setText( alias);
    QPixmap pm( icon);
    ui->lblIcon->setPixmap( pm);
}

QString Tile::alias()
{
    return ui->lblAlias->text();
}

void Tile::setStep( Step* step)
{
    setParent( step);
    m_step = step;
}

void Tile::createHeader( QString text, bool subhead)
{
    if ( text.isEmpty() ) return;
    FieldType ft = (subhead ? ftSubhead : ftHeader);
    Field* fld = new Field( m_main, this, "", text, m_xctrl, m_yctrl, ft, true);
    m_fields.append( fld);
    m_yctrl += 24;
    resize( width(), m_yctrl + 6);
    m_code->raise();
}

void Tile::createField( QString signal, QString alias, QString init, FieldType ft, bool vtab)
{
    Field* fld = new Field( m_main, this, signal, alias, m_xctrl, m_yctrl, ft, vtab);
    m_fields.append( fld);
    fld->initValue( init);
    if ( ft != ftConstant ) {
        if ( ft == ftLabel ) {
            if ( m_xctrl > 10 ) {
                m_xctrl = 10;
                m_yctrl += 20;
            }
            else
                m_xctrl = 140;
        }
        else
            m_yctrl += 20;
        fld->show();
        if ( m_xctrl > 10 )
            resize( width(), m_yctrl + 26);
        else
            resize( width(), m_yctrl + 6);
    }
    m_code->raise();
}

void Tile::buildTile()
{
    QObjectList cl = children();
    m_xctrl = 10;
    m_yctrl = 60;
    if ( m_codemode ) {
        for ( int i = 0; i < cl.count(); i++ )
            ((QWidget*)cl.at( i))->hide();
        ui->butCode->show();
        ui->butDelete->show();
        ui->butExpand->show();
        ui->lblAlias->show();
        ui->lblIcon->show();
        m_code->show();
        if ( m_codechanged )
            ui->butUnlock->show();
        resize( width(), (m_expanded ? 400 : 200));
        m_code->resize( this->width() - 19, this->height() - 55);
    }
    else {
        for ( int i = 0; i < cl.count(); i++ )
            ((QWidget*)cl.at( i))->show();
        m_code->hide();
        if ( m_step->isConfig() )
            ui->butExpand->hide();
        if ( !m_codechanged )
            ui->butUnlock->hide();
        for ( int i = 0; i < m_fields.count(); i++ ) {
            Field* fld = m_fields.at( i);

            if ( fld->vTab() && ((fld->type() != ftExpanded) || m_expanded) )
                m_yctrl += 8;

            fld->move( fld->x(), m_yctrl);

            if ( fld->type() == ftHeader )
                m_yctrl += 4;

            if ( fld->type() != ftConstant && ((fld->type() != ftExpanded) || m_expanded) ) {
                m_yctrl += 20;
                fld->show();
                if ( m_xctrl > 10 )
                    resize( width(), m_yctrl + 26);
                else
                    resize( width(), m_yctrl + 12);
            }
            else
                fld->hide();
            fld->setReadonly( m_codechanged);
        }
    }

    ui->butExpand->move( 136, height() - 10);
}

void Tile::flushField()
{
    if ( m_xctrl > 10 ) {
        m_xctrl = 10;
        m_yctrl += 20;
    }
}

void Tile::initField( QString alias, QString value, int fieldix)
{
    for ( int i = fieldix; i < m_fields.count(); i++ )
        if ( m_fields.at( i)->alias() == alias ) {
            m_fields.at( i)->initValue( value);
            break;
        }
}

Field* Tile::field( QString alias)
{
    for ( m_ix = 0; m_ix < m_fields.count(); m_ix++ )
        if ( m_fields.at( m_ix)->alias() == alias )
            return m_fields.at( m_ix);
    return NULL;
}

Field* Tile::firstField()
{
    m_ix = 0;
    if ( !m_fields.count() ) return NULL;
    return m_fields.at( 0);
}

Field* Tile::nextField()
{
    m_ix++;
    if ( m_ix >= m_fields.count() ) return NULL;
    return m_fields.at( m_ix);
}

QString Tile::findSignalKey( QString signal)
{
    Field* field = firstField();
    while ( field ) {
        if ( (alias() + " > " + field->alias()) == signal )
            return field->key();
        field = nextField();
    }
    return "";
}

QString Tile::findKeySignal( QString key)
{
    Field* field = firstField();
    while ( field ) {
        if ( field->key() == key )
            return alias() + " > " + field->alias();
        field = nextField();
    }
    return "";
}

Template* Tile::templ()
{
    return m_templ;
}

Step* Tile::step()
{
    return m_step;
}

void Tile::setPos( int x, int y)
{
    m_xpos = x;
    m_ypos = y;
}

void Tile::scrollTo( int scroll)
{
    move( m_xpos, m_ypos + scroll);
}

bool Tile::isDeviceUsed( QString alias)
{
    alias += " > ";
    for ( int i = 0; i < m_fields.count(); i++ )
        if ( m_fields.at( i)->value().startsWith( alias) )
            return true;
    return false;
}

void Tile::resetCode()
{
    m_templ->m_code->tabs( 0);
    if ( step()->isConfig() ) {
        QStringList sl; // dummy
        m_oldcode = "//BLOCK: INCLUDE/IMPORT FILES\n\n";
        m_oldcode += m_templ->m_code->writeInclude( sl);
        m_oldcode += "\n//BLOCK: GLOBAL DECLARATIONS\n\n";
        m_oldcode += m_templ->m_code->writeGlobal();
        m_oldcode += "\n//BLOCK: GLOBAL ROUTINES\n\n";
        m_oldcode += m_templ->m_code->writeRoutine();
        m_oldcode += "\n//BLOCK: GENERAL SETUP\n\n";
        m_oldcode += m_templ->m_code->writeSetup();
        m_oldcode += "\n//BLOCK: INITIALISE STEP\n\n";
        m_oldcode += m_templ->m_code->writeStepInit();
        m_oldcode += "\n//BLOCK: SENSOR UPDATE\n\n";
        m_oldcode += m_templ->m_code->writeSensorUpdate();
    }
    else {
        m_oldcode = "//BLOCK: DATA FEEDTHROUGH\n\n";
        m_oldcode += m_templ->m_code->writeDataFeedthrough( m_main->initVariant(), this);
        m_oldcode += "\n//BLOCK: FUNCTION UPDATE\n\n";
        m_oldcode += m_templ->m_code->writeFunctionUpdate();
        m_oldcode += "\n//BLOCK: ACTUATOR UPDATE\n\n";
        m_oldcode += m_templ->m_code->writeActuatorUpdate();
    }
    m_code->clear();
    m_code->appendCpp( m_oldcode);
    m_code->moveCursor( QTextCursor::Start);
    m_oldpos = 0;
    m_codechanged = false;
    ui->butUnlock->hide();
}

void Tile::setCode( QString code, int cursorpos)
{
    QTextCursor tc = m_code->textCursor();
    int pos;
    bool state = m_code->blockSignals( true);
    if ( cursorpos < 0 || cursorpos >= code.length() )
        pos = tc.position();
    else
        pos = cursorpos;
    m_code->clear();
    m_code->appendCpp( code);
    tc.setPosition( pos);
    m_code->setTextCursor( tc);
    m_code->blockSignals( state);
}

void Tile::on_butExpand_clicked()
{
    m_expanded = !m_expanded;
    ui->butExpand->setText( m_expanded ? "▲" : "▼");
    buildTile();
    step()->modelAllTiles();
    ui->lblIcon->setFocus();
}

void Tile::on_butCode_clicked()
{
    m_codemode = !m_codemode;
    if ( !m_codechanged )
        resetCode();
    buildTile();
    step()->modelAllTiles();
}

void Tile::on_butUnlock_clicked()
{
// CURRENT RELEASE DOES NOT SUPPORT EDITING TILE CODE
// PREPERATION FOR NEXT RELEASE

    if ( m_main->message( QMessageBox::Warning, tr( "De code is handmatig aangepast.\nAl deze aanpassingen zullen verloren gaan."),
                          tr( "Toch ontgrendelen?")) == QMessageBox::Yes ) {
        resetCode();
        buildTile();
    }
}

void Tile::on_edtCode_textChanged()
{
// CURRENT RELEASE DOES NOT SUPPORT EDITING TILE CODE
setCode( m_oldcode, m_code->textCursor().position());
return;

// PREPERATION FOR NEXT RELEASE

    m_codechanged = true;
    ui->butUnlock->show();

    QString block = "//BLOCK:";
    QString str = m_code->toPlainText();
    QTextCursor tc = m_code->textCursor();
    int pln, pos = tc.position();

    tc.movePosition( QTextCursor::StartOfLine);
    pln = tc.position();
    if ( m_oldcode.mid( pln, 8).contains( block) ) {
        setCode( m_oldcode, pos);
        return;
    }

    setCode( str, pos);

    m_oldcode = str;
    m_oldpos = pos;
}

void Tile::on_edtCode_selectionChanged()
{
// CURRENT RELEASE DOES NOT SUPPORT EDITING TILE CODE
// PREPERATION FOR NEXT RELEASE

    if ( m_blockselection ) {
        m_blockselection = false;
        return;
    }

    QString block = "//BLOCK:";
    QString str = m_code->toPlainText();
    QTextCursor tc = m_code->textCursor();
    int pstart = tc.selectionStart();
    int pend = tc.selectionEnd();
    int pos = tc.position();

    tc.setPosition( pstart);
    tc.movePosition( QTextCursor::StartOfLine);
    pstart = tc.position();
    tc.setPosition( pend);
    tc.movePosition( QTextCursor::EndOfLine);
    pend = tc.position();
    str = str.mid( pstart, pend - pstart);

    if ( str.contains( block) ) {
        m_blockselection = true;
        tc.setPosition( pos);
        m_code->setTextCursor( tc);
    }
}
