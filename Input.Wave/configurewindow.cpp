#include <windows.h>

#include "configurewindow.h"
#include "ui_configurewindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\DWM.h"
#include "..\include\Interfaces.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Input.Wave::ConfigureWindow")

//---------------------------------------------------------------------------//

ConfigureWindow::ConfigureWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfigureWindow)
{
    ui->setupUi(this);

    EnableAeroGlass(this);
}

ConfigureWindow::~ConfigureWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------//

