#-------------------------------------------------
#
# Project created by QtCreator 2013-06-29T06:53:22
#
#-------------------------------------------------

QT       += widgets

TARGET = SimplePlayer
TEMPLATE = lib

DEFINES += SIMPLEPLAYER_LIBRARY

SOURCES += \
    ClassFactory.cpp \
    PluginProperty.cpp \
    DllMain.cpp \
    Plugin.cpp \
    ../include/LockModule.cpp \
    mainwindow.cpp \
    ../include/DWM.cpp \
    ../include/DebugPrint.cpp

HEADERS += \
    ../include/Interfaces.h \
    ../include/LockModule.h \
    ../include/PluginProperty.h \
    ../include/ClassFactory.h \
    ../include/DebugPrint.h \
    mainwindow.h \
    ../include/DWM.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

DEF_FILE += ../CubeMelon.def

RES_FILE += Plugin.res

CONFIG += dll

OTHER_FILES += \
    Plugin.res \
    ../CubeMelon.def

FORMS += \
    mainwindow.ui
