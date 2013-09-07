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
    configurationwindow.cpp \
    ClassFactory.cpp \
    PropertyStore.cpp \
    DllMain.cpp \
    Wasapi.cpp

HEADERS += \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/Interfaces.h \
    ../include/LockModule.h \
    ../include/ClassFactory.h \
    ../include/PropertyStore.h \
    ../include/IOPlugin.h \
    configurationwindow.h \
    Wasapi.h \

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
