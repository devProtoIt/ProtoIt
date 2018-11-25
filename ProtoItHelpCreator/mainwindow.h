#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>
#include <QListWidgetItem>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:

    void closeEvent( QCloseEvent *event);

private slots:
    void on_btnFile_clicked();
    void on_btnModel_clicked();
    void on_lstScript_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_btnHelp1_clicked();
    void on_btnHelp2_clicked();
    void on_edtInfo_textChanged();
    void on_edtScript_textChanged();

    void on_space();
    void on_bold();
    void on_italic();
    void on_underscore();
    void on_break();
    void on_lessthen();
    void on_greaterthen();
    void on_degrees();
    void on_quotes();

private:
    Ui::MainWindow *ui;

    int message( QMessageBox::Icon icon, QString info, QString ask = "");
    QString makeHtml();

    QString     m_platform;
    QString     m_author;
    QString     m_year;
    QString     m_model;
    QString     m_icon;
    QString     m_path;

    QStringList m_help;
    int         m_ixhelp;

    QString     m_html;
    bool        m_saved;
};

#endif // MAINWINDOW_H
