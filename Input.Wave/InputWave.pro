#-------------------------------------------------
#
# Project created by QtCreator 2013-09-03T23:46:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Input.Wave
TEMPLATE = lib

SOURCES += \
    ../include/DWM.cpp \
    ../include/DebugPrint.cpp \
    ../include/LockModule.cpp \
    configurewindow.cpp \
    ClassFactory.cpp \
    DllMain.cpp \
    IOComponent.cpp \
    ComponentProperty.cpp

HEADERS += \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/Interfaces.h \
    ../include/LockModule.h \
    ../include/ComponentProperty.h \
    ../include/ClassFactory.h \
    ../include/IOComponent.h \
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

RES_FILE += Component.res

CONFIG += dll

OTHER_FILES += \
    ../CubeMelon.def \
    Component.res
