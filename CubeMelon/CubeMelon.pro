#-------------------------------------------------
#
# Project created by QtCreator 2013-06-06T02:35:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CubeMelon
TEMPLATE = app

SOURCES +=\
    ../include/DWM.cpp \
    ../include/DebugPrint.cpp \
    ../include/LockModule.cpp \
    ../include/ComponentBase.cpp \
    ../include/Module.PropManager.cpp \
    ClassFactory.cpp \
    CompManager.cpp \
    mainwindow.cpp \
    main.cpp\
    Host.cpp

HEADERS +=\
    ../include/ComPtr.h \
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/LockModule.h \
    ../include/Functions.h \
    ../include/Interfaces.h \
    ../include/ClassFactory.h \
    ../include/ComponentBase.h \
    ../include/PropManager.h \
    CompManager.h \
    mainwindow.h \
    Host.h

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

RES_FILE += CubeMelon.res

OTHER_FILES += \
    CubeMelon.res \
    ../CubeMelon.def
    ../CubeMelon.def
