#-------------------------------------------------
#
# Project created by QtCreator 2014-05-05T23:31:57
#
#-------------------------------------------------

QT += core gui
QT += serialport
QT += widgets
QT += multimedia
QT += multimediawidgets
QT += webkitwidgets
QT += bluetooth

TARGET = ProtoIt
TEMPLATE = app
DESTDIR = "../ProtoIt_Git/ProtoIt Windows Release/"

SOURCES += main.cpp\
    mainwindow.cpp \
    step.cpp \
    template.cpp \
    tileid.cpp \
    tile.cpp \
    codewriter.cpp \
    history.cpp \
    field.cpp \
    info.cpp \
    qplatform.cpp \
    renamestep.cpp \
    helpviewer.cpp \
    deviceviewer.cpp \
    communicator.cpp \
    serialmonitor.cpp \
    serialselect.cpp \
    historyreport.cpp \
    platformmanager.cpp \
    codetextedit.cpp

HEADERS  += mainwindow.h \
    step.h \
    template.h \
    tileid.h \
    tile.h \
    codewriter.h \
    history.h \
    field.h \
    info.h \
    qplatform.h \
    renamestep.h \
    helpviewer.h \
    deviceviewer.h \
    communicator.h \
    serialmonitor.h \
    serialselect.h \
    historyreport.h \
    platformmanager.h \
    codetextedit.h

FORMS    += mainwindow.ui \
    tileid.ui \
    tile.ui \
    field.ui \
    info.ui \
    qplatform.ui \
    renamestep.ui \
    helpviewer.ui \
    deviceviewer.ui \
    communicator.ui \
    serialmonitor.ui \
    serialselect.ui \
    historyreport.ui \
    platformmanager.ui

RESOURCES   += ProtoIt.qrc
TRANSLATIONS += ProtoIt_en.ts

RC_ICONS += ProtoIt.ico \
    ProtoItFile.ico
