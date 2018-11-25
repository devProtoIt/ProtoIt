#ifndef CODETEXTEDIT_H
#define CODETEXTEDIT_H

#include <QPlainTextEdit>
#include <QMouseEvent>

class CodeTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeTextEdit(QWidget *parent = 0);

    void appendCpp( const QString &text);

protected:

    void mouseMoveEvent( QMouseEvent *e);

private:

    QWidget* m_tile;
};

#endif // CODETEXTEDIT_H
