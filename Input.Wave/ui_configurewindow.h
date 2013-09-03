/********************************************************************************
** Form generated from reading UI file 'configurewindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGUREWINDOW_H
#define UI_CONFIGUREWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ConfigureWindow
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ConfigureWindow)
    {
        if (ConfigureWindow->objectName().isEmpty())
            ConfigureWindow->setObjectName(QStringLiteral("ConfigureWindow"));
        ConfigureWindow->resize(400, 300);
        menuBar = new QMenuBar(ConfigureWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        ConfigureWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ConfigureWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ConfigureWindow->addToolBar(mainToolBar);
        centralWidget = new QWidget(ConfigureWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        ConfigureWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ConfigureWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ConfigureWindow->setStatusBar(statusBar);

        retranslateUi(ConfigureWindow);

        QMetaObject::connectSlotsByName(ConfigureWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ConfigureWindow)
    {
        ConfigureWindow->setWindowTitle(QApplication::translate("ConfigureWindow", "ConfigureWindow", 0));
    } // retranslateUi

};

namespace Ui {
    class ConfigureWindow: public Ui_ConfigureWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGUREWINDOW_H
