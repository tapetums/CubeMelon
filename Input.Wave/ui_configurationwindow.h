/********************************************************************************
** Form generated from reading UI file 'configurationwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGURATIONWINDOW_H
#define UI_CONFIGURATIONWINDOW_H

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

class Ui_ConfigurationWindow
{
public:
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ConfigurationWindow)
    {
        if (ConfigurationWindow->objectName().isEmpty())
            ConfigurationWindow->setObjectName(QStringLiteral("ConfigurationWindow"));
        ConfigurationWindow->resize(400, 300);
        centralWidget = new QWidget(ConfigurationWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        ConfigurationWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ConfigurationWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 400, 24));
        ConfigurationWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ConfigurationWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ConfigurationWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ConfigurationWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ConfigurationWindow->setStatusBar(statusBar);

        retranslateUi(ConfigurationWindow);

        QMetaObject::connectSlotsByName(ConfigurationWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ConfigurationWindow)
    {
        ConfigurationWindow->setWindowTitle(QApplication::translate("ConfigurationWindow", "ConfigureWindow", 0));
    } // retranslateUi

};

namespace Ui {
    class ConfigurationWindow: public Ui_ConfigurationWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGURATIONWINDOW_H
