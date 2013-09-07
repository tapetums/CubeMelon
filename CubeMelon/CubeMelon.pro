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
    main.cpp\
    mainwindow.cpp \
    ComponentManager.cpp \
    Host.cpp \
    PropertyStore.cpp

HEADERS +=\
    ../include/DWM.h \
    ../include/DebugPrint.h \
    ../include/Interfaces.h \
    ../include/LockModule.h \
    ../include/ComPtr.h \
    mainwindow.h \
    ComponentManager.h \
    Host.h \
    ../include/PropertyStore.h

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

RES_FILE += CubeMelon.res

OTHER_FILES += \
    CubeMelon.res
