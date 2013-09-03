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
    ../include/LockModule.cpp \
    main.cpp\
    mainwindow.cpp \
    PluginManager.cpp \
    Plugin.cpp \
    PluginProperty.cpp \
    ../include/DebugPrint.cpp

HEADERS  +=\
    ../include/DWM.h \
    ../include/Interfaces.h \
    mainwindow.h \
    ../include/LockModule.h \
    ../include/PluginManager.h \
    ../include/PluginProperty.h \
    ../include/Plugin.h \
    ../include/DebugPrint.h

FORMS    += mainwindow.ui

RES_FILE += CubeMelon.res

OTHER_FILES += \
    CubeMelon.res
