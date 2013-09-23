// CubeMelon.mainwindow.cpp

#define NOMINMAX /// QTBUG-30839

#include <windows.h>
#include <strsafe.h>
#include <dwmapi.h>

#include <QDateTime>

#include "..\include\DWM.h"
#include "..\include\DebugPrint.h"
#include "..\include\Interfaces.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("CubeMelon.MainWindow")

//---------------------------------------------------------------------------//

struct MainWindow::Impl
{
    Impl();
    ~Impl();
};

//---------------------------------------------------------------------------//

MainWindow::Impl::Impl()
{
}

//---------------------------------------------------------------------------//

MainWindow::Impl::~Impl()
{
}

//---------------------------------------------------------------------------//

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    ui->setupUi(this);
    ui->tabWidget->setTabText(0, QString("Plugins"));
    ui->tabWidget->setTabText(1, QString("Console"));

    EnableAeroGlass(this);

    DebugPrintLn(NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

MainWindow::~MainWindow()
{
    DebugPrintLn(NAME TEXT("::Destructor() begin"));

    delete ui;

    DebugPrintLn(NAME TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

void MainWindow::addConsoleText(const wchar_t* text)
{
    WCHAR buf[1024];

    auto time = QTime::currentTime();
    ::StringCchPrintf
    (
        buf, 1024,
        TEXT("%02d:%02d:%02d;%04d> %s"),
        time.hour(), time.minute(), time.second(), time.msec(),
        text
    );
    ui->plainTextEdit->appendPlainText(QString::fromUtf16((const ushort*)buf));
}

//---------------------------------------------------------------------------//

void MainWindow::resizeEvent(QResizeEvent* event)
{
    DebugPrintLn(NAME TEXT("::resizeEvent() begin"));

    const int m_bar = ui->menuBar->height();
    const int s_bar = ui->statusBar->height();

    auto w = event->size().width();
    auto h = event->size().height();
    ui->tabWidget->setGeometry(0, m_bar, w, h - m_bar - s_bar -1);
    ui->statusBar->setGeometry(0, h - s_bar, w, s_bar);

    w = ui->tabWidget->width();
    h = ui->tabWidget->height();
    ui->treeWidget->setGeometry(0, 0, w, h);
    ui->plainTextEdit->setGeometry(0, 0, w, h);

    DebugPrintLn(NAME TEXT("::resizeEvent() end"));

    QMainWindow::resizeEvent(event);
}

//---------------------------------------------------------------------------//

void MainWindow::addListItem(CubeMelon::ICompAdapter* adapter)
{
    DebugPrintLn(NAME TEXT("::addListItem() begin"));

    if ( nullptr == adapter )
    {
        return;
    }

    adapter->AddRef();

    WCHAR buf[256];

    auto item = new QTreeWidgetItem;

    auto clsid = adapter->ClassID();
    ::StringCchPrintf
    (
        buf, 256,
        TEXT("{ %00000000X-%0000X-%0000X-%00X%00X-%00X%00X%00X%00X%00X%00X }"),
        clsid.Data1, clsid.Data2, clsid.Data3,
        clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
        clsid.Data4[4], clsid.Data4[5], clsid.Data4[6], clsid.Data4[7]
    );
    item->setText(4, QString::fromUtf16((const ushort*)buf));

    item->setText(0, QString::fromUtf16((const ushort*)adapter->Name()));

    item->setText(2, QString::fromUtf16((const ushort*)adapter->Description()));

    item->setText(3, QString::fromUtf16((const ushort*)adapter->Copyright()));

    auto vi = adapter->Version();
    ::StringCchPrintf
    (
        buf, 256,
        TEXT("%d.%d.%d.%d"),
        vi->major, vi->minor, vi->revision, vi->stage
    );
    item->setText(1, QString::fromUtf16((const ushort*)buf));

    ui->treeWidget->addTopLevelItem(item);

    auto count = ui->treeWidget->topLevelItemCount();
    ::StringCchPrintf
    (
        buf, 256,
        (count < 2) ? TEXT("%d component") : TEXT("%d components"),
        count
    );
    ui->statusBar->showMessage(QString::fromUtf16((const ushort*)buf));

    adapter->Release();

    DebugPrintLn(NAME TEXT("::addListItem() end"));
}

//---------------------------------------------------------------------------//

void MainWindow::removeListItem()
{
}

//---------------------------------------------------------------------------//

void MainWindow::clearList()
{
}

//---------------------------------------------------------------------------//

// CubeMelon.mainwindow.cpp