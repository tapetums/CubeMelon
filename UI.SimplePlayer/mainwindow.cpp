// UI.SimplePlayer.mainwindow.cpp

#include <QMimeData>
#include <QDragEnterEvent>
#include <QResizeEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUrl>

#include <windows.h>
#include <strsafe.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "..\include\DWM.h"
#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"

//---------------------------------------------------------------------------//

#define NAME        TEXT("UI.SimplePlayer.MainWindow")
#define MSGOBJ_NAME TEXT("UI.SimplePlayer.MsgObject")

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const wchar_t* MSG_ADD_TO_PLAYLIST;

//---------------------------------------------------------------------------//

class MsgObject : public IMsgObject
{
public:
    explicit MsgObject(IComponent* sender, WCHAR* path);
    ~MsgObject();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    LPCWSTR     __stdcall Name() const override;
    LPCWSTR     __stdcall Message() const override;
    IComponent* __stdcall Sender() const override;
    IComponent* __stdcall Listener() const override;
    size_t      __stdcall DataCount() const override;
    LPCWSTR     __stdcall DataName(size_t index = 0) const override;
    size_t      __stdcall DataSize(size_t index = 0) const override;
    DATATYPE    __stdcall DataType(size_t index = 0) const override;
    LPVOID      __stdcall Data(size_t index = 0) const override;

protected:
    ULONG m_cRef;

private:
    IComponent* m_sender;
    WCHAR*      m_data;
    size_t      m_size;
};

//---------------------------------------------------------------------------//

MsgObject::MsgObject
(
    IComponent* sender, WCHAR* path
)
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::Constructor() begin"));

    m_cRef = 0;

    m_sender = sender;
    m_data   = path;
    m_size   = path ? sizeof(WCHAR) * (1 + wcslen(path)) : 0;

    if ( m_sender )
    {
        m_sender->AddRef();
    }

    this->AddRef();

    DebugPrintLn(MSGOBJ_NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

MsgObject::~MsgObject()
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::Destructor() begin"));

    if ( m_sender )
    {
        m_sender->Release();
        m_sender = nullptr;
    }

    DebugPrintLn(MSGOBJ_NAME TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall MsgObject::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMsgObject) )
    {
        *ppvObject = static_cast<IMsgObject*>(this);
    }
    else
    {
        DebugPrintLn(TEXT("No such an interface"));
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(MSGOBJ_NAME TEXT("::QueryInterface() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall MsgObject::AddRef()
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(MSGOBJ_NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall MsgObject::Release()
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(MSGOBJ_NAME TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall MsgObject::Name() const
{
    return MSGOBJ_NAME;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall MsgObject::Message() const
{
    return MSG_ADD_TO_PLAYLIST;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall MsgObject::Sender() const
{
    return m_sender;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall MsgObject::Listener() const
{
    return nullptr;
}

//---------------------------------------------------------------------------//

size_t __stdcall MsgObject::DataCount() const
{
    return 1;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall MsgObject::DataName(size_t index) const
{
    return TEXT("Path");
}

//---------------------------------------------------------------------------//

size_t __stdcall MsgObject::DataSize(size_t index) const
{
    return m_size;
}

//---------------------------------------------------------------------------//

DATATYPE __stdcall MsgObject::DataType(size_t index) const
{
    return TYPE_WSTRING;
}

//---------------------------------------------------------------------------//

LPVOID __stdcall MsgObject::Data(size_t index) const
{
    return m_data;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

void AddItem(QTreeWidget* tree, LPCWSTR path)
{
    WCHAR file_name[MAX_PATH];

    auto tmp = ::PathFindFileName(path);
    ::PathRemoveExtension(tmp);
    ::StringCchCopy(file_name, MAX_PATH, tmp);

    const auto count = 1+ tree->topLevelItemCount();

    auto item = new QTreeWidgetItem;
    item->setText(0, QString("%1").arg(count, 2, 10, QChar('0')).toUpper());
    item->setText(1, QString::fromUtf16((const ushort*)file_name));

    tree->addTopLevelItem(item);
}

//---------------------------------------------------------------------------//

struct MainWindow::Impl
{
    CubeMelon::IComponent* owner;
    QImage image;
};

//---------------------------------------------------------------------------//

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    pimpl = new Impl;
    pimpl->owner = nullptr;

    ui->setupUi(this);
    ui->treeWidget->setColumnWidth(0, 48);
    ui->treeWidget->setColumnWidth(4, 64);
    ui->treeWidget->setColumnWidth(5, 96);

    EnableAeroGlass(this);
    EnableAeroGlass(ui->treeWidget);

    DebugPrintLn(NAME  TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

MainWindow::~MainWindow()
{
    DebugPrintLn(NAME  TEXT("::Destructor() begin"));

    delete pimpl;
    pimpl = nullptr;

    delete ui;

    DebugPrintLn(NAME  TEXT("::Destructor() end"));
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
        pimpl->owner = nullptr;
    }

    DebugPrintLn(NAME TEXT("::closeEvent() end"));

    QMainWindow::closeEvent(event);
}

//---------------------------------------------------------------------------//

void MainWindow::dragEnterEvent(QDragEnterEvent * event)
{
    DebugPrintLn(NAME TEXT("::dragEnterEvent() begin"));

    event->acceptProposedAction();

    DebugPrintLn(NAME TEXT("::dragEnterEvent() end"));
}

//---------------------------------------------------------------------------//

void MainWindow::dropEvent(QDropEvent * event)
{
    DebugPrintLn(NAME TEXT("::dropEvent() begin"));

    if ( event->mimeData()->hasUrls() )
    {
        auto const owner = pimpl->owner;
        if ( owner )
        {
            const auto urls = event->mimeData()->urls();
            const auto itE = urls.end();
            auto it = urls.begin();

            DebugPrintLn(TEXT("Checking all files..."));
            while ( it != itE )
            {
                auto path = (WCHAR*)it->path().utf16();
                #ifdef WIN32
                if ( path[0] == '/' && path[1] != '\0' && path[2] == ':' )
                    {
                        ++path;
                    }
                #endif
                DebugPrintLn(TEXT(". %s"), path);

                auto hr = owner->Notify(new CubeMelon::MsgObject(owner, path));
                if ( SUCCEEDED(hr) )
                {
                    DebugPrintLn(TEXT("Adding item..."));
                    {
                        AddItem(ui->treeWidget, path);
                    }
                    DebugPrintLn(TEXT("Added item"));
                }
                DebugPrintLn(TEXT(". Next file"));
                ++it;
            }
            DebugPrintLn(TEXT("Checked all files"));
        }
    }

    DebugPrintLn(NAME TEXT("::dropEvent() end"));
}

//---------------------------------------------------------------------------//

void MainWindow::paintEvent(QPaintEvent *event)
{
//    QPainter painter(this);
//    painter.fillRect(event->rect(), QColor(0, 0, 0, 0));
}

//---------------------------------------------------------------------------//

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DebugPrintLn(NAME TEXT("::resizeEvent() begin"));

    const auto w = ui->centralwidget->width();
    const auto h = ui->centralwidget->height();

    auto cw0 = ui->treeWidget->columnWidth(0);
    auto cw4 = ui->treeWidget->columnWidth(4);
    auto cw5 = ui->treeWidget->columnWidth(5);
    auto cw_sum = cw0 + cw4 + cw5;

    ui->treeWidget->setGeometry(0, 0, w, h - 24 - 20 - 8);
    ui->treeWidget->setColumnWidth(0, cw0);
    ui->treeWidget->setColumnWidth(1, 3 * (w - cw_sum) / 8);
    ui->treeWidget->setColumnWidth(2, 2 * (w - cw_sum) / 8);
    ui->treeWidget->setColumnWidth(3, 3 * (w - cw_sum) / 8);
    ui->treeWidget->setColumnWidth(4, cw4);
    ui->treeWidget->setColumnWidth(5, cw5);

    ui->lcdNumber->setGeometry((w - 128)/2, h -20 -24, 64, 24);
    ui->horizontalSlider->setGeometry(0, h - 20, w, 20);

    DebugPrintLn(NAME TEXT("::resizeEvent() end"));

    QMainWindow::resizeEvent(event);
}

//---------------------------------------------------------------------------//

// UI.SimplePlayer.mainwindow.cpp