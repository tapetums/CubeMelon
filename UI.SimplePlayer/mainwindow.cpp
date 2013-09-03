#include <windows.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\DWM.h"
#include "..\include\Interfaces.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("SimplePlayer::MainWindow")

//---------------------------------------------------------------------------//

struct MainWindow::Impl
{
    IPlugin* owner;
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

void MainWindow::setOwner(IPlugin *owner)
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
