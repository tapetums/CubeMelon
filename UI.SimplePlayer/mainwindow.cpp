#include <windows.h>

#include <QResizeEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\DWM.h"
#include "..\include\Interfaces.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("UI.SimplePlayer.MainWindow")

//---------------------------------------------------------------------------//

struct MainWindow::Impl
{
    CubeMelon::IComponent* owner;
};

//---------------------------------------------------------------------------//

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    pimpl = new Impl;
    pimpl->owner = nullptr;

    EnableAeroGlass(this);
}

//---------------------------------------------------------------------------//

MainWindow::~MainWindow()
{
    delete pimpl;
    pimpl = nullptr;

    delete ui;
}

//---------------------------------------------------------------------------//

void MainWindow::setOwner(CubeMelon::IComponent *owner)
{
    pimpl->owner = owner;
}

//---------------------------------------------------------------------------//

void MainWindow::closeEvent(QCloseEvent *event)
{
    DebugPrintLn(NAME TEXT("::closeEvent() begin"));

    if ( pimpl->owner )
    {
        pimpl->owner->Stop();
    }

    DebugPrintLn(NAME TEXT("::closeEvent() end"));

    QMainWindow::closeEvent(event);
}

//---------------------------------------------------------------------------//

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DebugPrintLn(NAME TEXT("::resizeEvent() begin"));

    const auto w = event->size().width();
    const auto h = event->size().height();

    ui->horizontalSlider->setGeometry(0, h - 20, w, 20);

    DebugPrintLn(NAME TEXT("::resizeEvent() end"));

    QMainWindow::resizeEvent(event);
}

//---------------------------------------------------------------------------//

