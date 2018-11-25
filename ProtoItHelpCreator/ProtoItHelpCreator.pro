#-------------------------------------------------
#
# Project created by QtCreator 2015-09-02T19:42:03
#
#-------------------------------------------------

QT += core gui
QT += widgets
QT += multimedia
QT += multimediawidgets
QT += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ProtoItHelpCreator
TEMPLATE = app
DESTDIR = ../ProtoIt Git/ProtoIt Release/


SOURCES += main.cpp\
        mainwindow.cpp \
    dlghelp.cpp

HEADERS  += mainwindow.h \
    dlghelp.h

FORMS    += mainwindow.ui \
    dlghelp.ui

RC_ICONS += ProtoItHelpCreator.ico
