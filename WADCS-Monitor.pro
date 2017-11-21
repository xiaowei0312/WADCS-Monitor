#-------------------------------------------------
#
# Project created by QtCreator 2017-11-14T17:17:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WADCS-Monitor
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \ 
    qextserialbase.cpp \
    qextserialenumerator.cpp \
    qextserialport.cpp \
    win_qextserialport.cpp \ 
    myserialport.cpp \
    mysendthread.cpp \
    myreceivethread.cpp \
    stringutil.cpp \
    myprotoparsethread.cpp
HEADERS  += mainwindow.h \
    qcustomplot.h \ 
    qextserialbase.h \
    qextserialenumerator.h \
    qextserialport.h \
    win_qextserialport.h \ 
    myserialport.h \
    mysendthread.h \
    myreceivethread.h \
    stringutil.h \
    myprotoparsethread.h \
    common.h

FORMS    += \
    mainwindow.ui

RESOURCES += \
    wadcs.qrc
