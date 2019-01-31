#-------------------------------------------------
#
# Project created by QtCreator 2019-01-29T11:56:49
#
#-------------------------------------------------

QT       += core gui printsupport serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SpectrumMap
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    plot/qcustomplot.cpp \
    qhpgdevicelib.cpp \
    indicator/indicator.cpp \
    masterthread.cpp \
    movetable.cpp

HEADERS  += mainwindow.h \
    plot/qcustomplot.h \
    qhpgdevicelib.h \
    indicator/indicator.h \
    masterthread.h \
    movetable.h

FORMS    += mainwindow.ui

DISTFILES += \
    "must be placed near exe/RCOM4_CTRL.dll" \
    "must be placed near exe/Calibration.ini" \
    indicator/indicatorLoading.gif \
    indicator/indicatorOff.png \
    indicator/indicatorOn.png \
    indicator/indicatorOn2.png

SUBDIRS += \
    indicator/Indicator.pro

RESOURCES += \
    indicator/indicatorpicturesresource.qrc

RC_FILE = icon.rc
