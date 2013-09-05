// mainwindow.cpp

#define NOMINMAX /// QTBUG-30839

#include <windows.h>
#include <strsafe.h>
#include <dwmapi.h>

#include <QDateTime>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "..\include\DWM.h"
#include "..\include\Interfaces.h"

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
    ui->setupUi(this);
    ui->tabWidget->setTabText(0, QString("Plugins"));
    ui->tabWidget->setTabText(1, QString("Console"));

    EnableAeroGlass(this);
}

//---------------------------------------------------------------------------//

MainWindow::~MainWindow()
{
    delete ui;
}

//---------------------------------------------------------------------------//

void MainWindow::addConsoleText(LPCWSTR text)
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
    const int m_bar = ui->menuBar->height();
    const int s_bar = ui->statusBar->height();

    auto w = event->size().width();
    auto h = event->size().height();
    ui->tabWidget->setGeometry(0, m_bar, w, h - m_bar - s_bar);
    ui->statusBar->setGeometry(0, h - s_bar, w, s_bar);

    w = ui->tabWidget->width();
    h = ui->tabWidget->height();
    ui->treeWidget->setGeometry(0, 0, w, h);
    ui->plainTextEdit->setGeometry(0, 0, w, h);

}

//---------------------------------------------------------------------------//

void MainWindow::addListItem
(
    REFCLSID clsid, LPCWSTR  name,
    LPCWSTR  description, LPCWSTR  copyright, VersionInfo* vi
)
{
    WCHAR buf[256];

    auto item = new QTreeWidgetItem;

    ::StringCchPrintf
    (
        buf, 256,
        TEXT("{ %00000000X-%0000X-%0000X-%00X%00X-%00X%00X%00X%00X%00X%00X }"),
        clsid.Data1, clsid.Data2, clsid.Data3,
        clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
        clsid.Data4[4], clsid.Data4[5], clsid.Data4[6], clsid.Data4[7]
    );
    item->setText(4, QString::fromUtf16((const ushort*)buf));

    item->setText(0, QString::fromUtf16((const ushort*)name));

    item->setText(2, QString::fromUtf16((const ushort*)description));

    item->setText(3, QString::fromUtf16((const ushort*)copyright));

    ::StringCchPrintf
    (
        buf, 256,
        TEXT("%d.%d.%d %s"),
        vi->major, vi->minor, vi->revision, vi->stage
    );
    item->setText(1, QString::fromUtf16((const ushort*)buf));

    ui->treeWidget->addTopLevelItem(item);

    auto count = ui->treeWidget->topLevelItemCount();
    ::StringCchPrintf
    (
        buf, 256,
        (count < 2) ? TEXT("%d plugin") : TEXT("%d plugins"),
        count
    );
    ui->statusBar->showMessage(QString::fromUtf16((const ushort*)buf));
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

// mainwindow.cpp