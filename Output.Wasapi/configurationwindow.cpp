#include <windows.h>

#include "configurationwindow.h"
#include "ui_configurationwindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\DWM.h"
#include "..\include\Interfaces.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Output.Wasapi::ConfigurationWindow")

//---------------------------------------------------------------------------//

ConfigurationWindow::ConfigurationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ConfigurationWindow)
{
    ui->setupUi(this);

    EnableAeroGlass(this);
}

ConfigurationWindow::~ConfigurationWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------//

