#-------------------------------------------------
#
# Project created by QtCreator 2013-06-29T06:53:22
#
#-------------------------------------------------

QT       += widgets

TARGET = UI.SimplePlayer
TEMPLATE = lib

SOURCES += \
    ../include/DWM.cpp \
    ../include/DebugPrint.cpp \
    ../include/LockModule.cpp \
    mainwindow.cpp \
    DllMain.cpp \
    ClassFactory.cpp \
    ComponentProperty.cpp \
    Component.cpp

HEADERS += \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/Interfaces.h \
    ../include/LockModule.h \
    ../include/ComponentProperty.h \
    ../include/ClassFactory.h \
    mainwindow.h

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
