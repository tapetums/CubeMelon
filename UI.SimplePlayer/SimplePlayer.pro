#-------------------------------------------------
#
# Project created by QtCreator 2013-06-29T06:53:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UI.SimplePlayer
TEMPLATE = lib

SOURCES += \
    ../include/DWM.cpp \
    ../include/DebugPrint.cpp \
    ../include/LockModule.cpp \
    ../include/UIComponentBase.cpp \
    ../include/Module.PropManager.cpp \
    ClassFactory.cpp \
    mainwindow.cpp \
    DllMain.cpp \
    SimplePlayer.cpp

HEADERS += \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/LockModule.h \
    ../include/Interfaces.h \
    ../include/ClassFactory.h \
    ../include/ComponentBase.h \
    ../include/PropManager.h \
    mainwindow.h \
    SimplePlayer.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    mainwindow.ui

DEF_FILE += ../CubeMelon.def

RES_FILE += Component.res

CONFIG += dll

OTHER_FILES += \
    Component.res \
    ../CubeMelon.def
