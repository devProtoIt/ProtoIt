#include "codetextedit.h"
#include "tile.h"
#include "template.h"
#include "step.h"
#include <QScrollBar>

CodeTextEdit::CodeTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    m_tile = parent;
    setLineWrapMode( QPlainTextEdit::NoWrap);
    setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded);
    QScrollBar *hbar = horizontalScrollBar();
    QScrollBar *vbar = verticalScrollBar();
    switch ( ((Tile*)m_tile)->templ()->deviceType() ) {
        case dtActuator :   vbar->setStyleSheet( ACTUATOR_COLOR);
                            hbar->setStyleSheet( ACTUATOR_COLOR);
                            break;
        case dtSensor :     vbar->setStyleSheet( SENSOR_COLOR);
                            hbar->setStyleSheet( SENSOR_COLOR);
                            break;
        case dtFunction :   vbar->setStyleSheet( FUNCTION_COLOR);
                            hbar->setStyleSheet( FUNCTION_COLOR);
                            break;
    }
}

void CodeTextEdit::mouseMoveEvent( QMouseEvent *e)
{
    setToolTip( "");
    QString white = " ,.;:-+*/&^%$?!(){}[]<>\t\n\"'";
    if ( rect().contains( e->x(), e->y()) ) {
        QString alias = ((Tile*) m_tile)->alias();
        QTextCursor tc = cursorForPosition( QPoint( e->x(), e->y()));
        QString s = toPlainText();
        int pl, pr, len = s.length();
        pl = pr = tc.position();
        if ( pl < 0 || pl >= len ) return;
        while ( (pl >= 0) && !white.contains( s.at( pl)) ) pl--;
        while ( (pr < len) && !white.contains( s.at( pr)) ) pr++;
        pl++;
        if ( pl > pr ) return;
        s = s.mid( pl, pr - pl);
        if ( s.left( alias.length() + 1) == alias + '_' ) {
            VarList *vl = &((Tile*)m_tile)->templ()->m_vars;
            Var* var = vl->firstVar( vtAll);
            while ( var && (var->key != s) )
                var = vl->nextVar( vtAll);
            if ( var ) {
                switch ( var->vt ) {
                    case vtPort :       s = alias + " > " + var->alias + tr( "  (Poort)"); break;
                    case vtAddress :    s = alias + " > " + var->alias + tr( "  (Adres)"); break;
                    case vtPrivate :    s = alias + " > " + var->alias + tr( "  (Lokale declaratie)"); break;
                    case vtCalibrate :  s = alias + " > " + var->alias + tr( "  (Kalibratie)"); break;
                    case vtSignalIn :   s = alias + " > " + var->alias + tr( "  (Op programmategel)"); break;
                    case vtSignalOut :  s = alias + " > " + var->alias + tr( "  (Uitgezonden signaal)"); break;
                    case vtConstant :   s = alias + " > " + var->alias + tr( "  (Constant signaal)"); break;
                    case vtGlobal :     s = alias + " > " + var->alias + tr( "  (Globale declaratie)"); break;
                    default :           s = alias + " > " + var->alias + tr( "  (?)"); break;
                }
                setToolTip( s);
            }
        }
        else
            if ( s.indexOf( '_') > 0 )
                setToolTip( tr( "Niet dit apparaat"));
    }
}

void CodeTextEdit::appendCpp( const QString &cpp)
{
    /* NB
     * QPlainTextEdit does not support &lt; and &gt; special characters with appendHtml.
     * Therefore the < character is replaced by @ at first and after appendHtml back to <.
     */

    QString sep = " ()[].:->";
    QString oper = "@@=>>=!==||=&&=++=--=*=/=^=";
    QString white = " ;:()[]{}.->@@=>>=!==||=&&=++=--=*=/=^=\t\n\"'";
    QString type = "void Variant int uint uint_t long ulong ulong_t float double String char";
    QString flow = "if then else while do for switch case default break";

    int il, ir;
    QString lstr, rstr, mstr;
    QString cd, id, code = cpp;
    bool font, text = false;

    code.replace( '<', '@');
    il = ir = 0;

    while ( il < code.length() ) {

        font = false;
        lstr = code.left( il);
        cd = code.at( il);
        if ( (il + 1 < code.length()) &&
             (cd == "/" || cd == ":" || oper.contains( cd)) ) {
            if ( (cd == code.at( il + 1)) || oper.contains( code.at( il + 1)) )
            cd += code.at( il + 1);
        }

        if ( !id.isEmpty() && white.contains( cd) ) {
            lstr = lstr.left( lstr.length() - id.length());
            if ( type.contains( id) )
                lstr += "<font color=green>" + id + "</font>";
            else
            if ( flow.contains( id) )
                lstr += "<font color=brown>" + id + "</font>";
            else
            if ( cd == "(" )
                lstr += "<i>" + id + "</i>";
            else
                lstr += id;
            id = "";
        }

        if ( cd == " " ) {
            lstr += "&nbsp;";
            il = ir = il + 1;
        }
        else
        if ( cd == "\t" ) {
            lstr += "&nbsp;&nbsp;";
            il = ir = il + 1;
        }
        else
        if ( cd == "\n" ) {
            lstr += "<br/>";
            il = ir = il + 1;
        }
        else
        if ( cd == ";" ) {
            lstr += cd;
            il = ir = il + 1;
        }
        else
        if ( cd == "{" || cd == "}" ) {
            lstr += "<font color=red>";
            font = true;
            ir = il + 1;
        }
        else
        if ( cd == "//" || cd == "/*" ) {
            lstr += "<font color=gray>";
            font = true;
            rstr = code.right( code.length() - il);
            if ( cd == "//" )
                ir = rstr.indexOf( "\n") - 1;
            else
                ir = rstr.indexOf( "*/") + 1;
            if ( ir > 0 ) ir += il + 1;
            else ir = code.length();
        }
        else
        if ( cd == "\"" || cd == "'" ) {
            text = !text;
            if ( text )
                lstr += "<font color=gray>" + id;
            else
                font = true;
            ir = il + 1;
        }
        else
        if ( sep.contains( cd) ) {
            lstr += "<font color=brown>";
            font = true;
            ir = il + cd.length();
        }
        else
        if ( oper.contains( cd) ) {
            lstr += "<font color=brown>";
            font = true;
            ir = il + cd.length();
        }
        else {
            id += cd;
            il++;
            continue;
        }

        mstr = code.mid( il, ir - il);
        rstr = code.right( code.length() - ir);
        code = lstr + mstr;
        if ( font ) code += "</font>";
        il = code.length();
        code += rstr;
    }
    code = "<code>" + code + "</code>";

    appendHtml( code);

    code = toPlainText();
    QTextCursor tc = textCursor();
    while ( (il = code.indexOf( '@')) > 0 ) {
        // insert first to keep character color
        tc.setPosition( il + 1);
        tc.insertText( "<");
        tc.setPosition( il);
        tc.deleteChar();
        code[ il] = '<';
    }
}
