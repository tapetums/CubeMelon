#-------------------------------------------------
#
# Project created by QtCreator 2013-09-04T06:23:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Output.Wasapi
TEMPLATE = lib

SOURCES += \
    ../include/DWM.cpp \
    ../include/DebugPrint.cpp \
    ../include/LockModule.cpp \
    ../include/OutputComponentBase.cpp \
    ../include/Module.PropManager.cpp \
    ClassFactory.cpp \
    configurationwindow.cpp \
    DllMain.cpp \
    Wasapi.cpp \
    WorkerThread.cpp

HEADERS += \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/LockModule.h \
    ../include/Interfaces.h \
    ../include/ClassFactory.h \
    ../include/ComponentBase.h \
    ../include/PropManager.h \
    configurationwindow.h \
    Wasapi.h \
    WorkerThread.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    configurationwindow.ui

DEF_FILE += ../CubeMelon.def

RES_FILE += Component.res

CONFIG += dll

OTHER_FILES += \
    ../CubeMelon.def \
    Component.res
