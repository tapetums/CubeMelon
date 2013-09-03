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
    configurewindow.cpp \
    ClassFactory.cpp \
    PluginProperty.cpp \
    DllMain.cpp \
    IOPlugin.cpp

HEADERS += \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/Interfaces.h \
    ../include/LockModule.h \
    ../include/PluginProperty.h \
    ../include/ClassFactory.h \
    ../include/IOPlugin.h \
    configurewindow.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    configurewindow.ui

DEF_FILE += ../CubeMelon.def

RES_FILE += Plugin.res

CONFIG += dll

OTHER_FILES += \
    Plugin.res \
    ../CubeMelon.def
