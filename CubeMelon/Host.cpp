// CubeMelon.Host.cpp

#include <windows.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\ComponentBase.h"

#include "CompManager.h"
#include "mainwindow.h"

#include "Host.h"

//---------------------------------------------------------------------------//

extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component =
{ 0x1511e8d0, 0x55a8, 0x4cc2, { 0x94, 0x8b, 0x9f, 0xee, 0xe, 0x63, 0x92, 0x5a } };

extern const size_t      MDL_PROP_COUNT    = 5;
extern const wchar_t*    MDL_PROP_MGR_NAME = TEXT("CubeMelon.PropManager");
extern const wchar_t*    MDL_PROP_NAME     = TEXT("CubeMelon.Property");

extern const size_t      COMP_INDEX      = 0;
extern const wchar_t*    COMP_NAME       = TEXT("CubeMelon.Host");

extern const wchar_t*    PropName        = TEXT("CubeMelon");
extern const wchar_t*    PropDescription = TEXT("Component-based Application System");
extern const wchar_t*    PropCopyright   = TEXT("(C) 2010-2013 tapetums");
extern const VersionInfo PropVersion     = { 1, 0, 0, 0 };

//---------------------------------------------------------------------------//

struct Host::Impl
{
    Impl();
    ~Impl();

    CompManager* manager;
    IComponent*  child;
    MainWindow*  mwnd;
    CLSID        clsid_autorun;
};

//---------------------------------------------------------------------------//

Host::Impl::Impl()
{
    DebugPrintLn(TEXT("%s::Impl::Constructor() begin"), COMP_NAME);

    manager = new CompManager();
    child   = nullptr;
    mwnd    = nullptr;

    CLSID clsid =
    { 0x520e13ad, 0x3345, 0x4377, { 0xb2, 0xef, 0x68, 0x42, 0xea, 0x79, 0xb2, 0x5b } };
    clsid_autorun = clsid;//CLSID_NULL;

    DebugPrintLn(TEXT("%s::Impl::Constructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

Host::Impl::~Impl()
{
    DebugPrintLn(TEXT("%s::Impl::Destructor() begin"), COMP_NAME);

    if ( child )
    {
        child->Release();
        child = nullptr;
    }
    if ( mwnd )
    {
        delete mwnd;
        mwnd = nullptr;
    }
    if ( manager )
    {
        manager->Release();
        manager = nullptr;
    }

    DebugPrintLn(TEXT("%s::Impl::Destructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

Host::Host() : ComponentBase(nullptr)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_NAME);

    pimpl = new Impl;

    DebugPrintLn(TEXT("%s::Constructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

Host::~Host()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), COMP_NAME);

    this->Stop();

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(TEXT("%s::Destructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::GetInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
{
    DebugPrintLn(TEXT("%s::GetInstance() begin"), COMP_NAME);

    HRESULT hr;

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_ICompManager) )
    {
        pimpl->manager->AddRef();
        *ppvObject = pimpl->manager;
        hr = S_OK;
    }
    else if ( IsEqualCLSID(rclsid, CLSID_Component) )
    {
        hr = this->QueryInterface(riid, ppvObject);
    }
    else if ( pimpl->child && IsEqualCLSID(rclsid, pimpl->child->ClassID()) )
    {
        hr = pimpl->child->QueryInterface(riid, ppvObject);
    }
    else
    {
        auto ca = pimpl->manager->Find(rclsid);
        if ( ca )
        {
            hr = ca->CreateInstance(this, riid, ppvObject);
            ca->Release();
        }
        else
        {
            hr = CS_E_CLASS_NOTFOUND;
        }
    }

    DebugPrintLn(TEXT("%s::GetInstance() end"), COMP_NAME);
    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Start(LPVOID args, IComponent* listener)
{
    DebugPrintLn(TEXT("%s::Start() begin"), COMP_NAME);

    HRESULT hr;

    if ( m_state & STATE_ACTIVE )
    {
        DebugPrintLn(TEXT("Already started"));
        return S_FALSE;
    }
    if ( m_state & STATE_STARTING )
    {
        DebugPrintLn(TEXT("Now starting"));
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STARTING);

    // コンポーネントの読み込み
    DebugPrintLn(TEXT("Loading components..."));
    {
        WCHAR dir_path[MAX_PATH];
        ::GetModuleFileName(nullptr, dir_path, MAX_PATH);
        ::PathRemoveFileSpec(dir_path);
        ::PathAppend(dir_path, TEXT("\\components"));

        hr = pimpl->manager->LoadAll(dir_path);
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    DebugPrintLn(TEXT("Loaded components"));

    // メインウィンドウの起動
    #if defined(_DEBUG) || defined(DEBUG)
    DebugPrintLn(TEXT("Opening MainWindow..."));
    {
        pimpl->mwnd = new MainWindow;
        if ( pimpl->mwnd )
        {
            ICompAdapter* adapter = nullptr;
            auto count = pimpl->manager->ComponentCount();
            for ( size_t index = 0; index < count; ++index )
            {
                adapter = pimpl->manager->GetAt(index);
                if ( adapter )
                {
                    pimpl->mwnd->addListItem(adapter);
                    pimpl->mwnd->addConsoleText(adapter->Name());
                    adapter->Release();
                }
            }
            pimpl->mwnd->addConsoleText(TEXT("Ready"));
            pimpl->mwnd->show();
        }
    }
    DebugPrintLn(TEXT("Opened MainWindow"));
    #endif

    // 子コンポーネントの起動
    DebugPrintLn(TEXT("Executing component..."));
    {
        hr = this->GetInstance
        (
            pimpl->clsid_autorun, IID_IComponent, (void**)&pimpl->child
        );
        if ( nullptr == pimpl->child )
        {
            DebugPrintLn(TEXT("Component was not found"));
            return hr;
        }
        hr = pimpl->child->Start();
    }
    DebugPrintLn(TEXT("Executed component"));

    m_state = (STATE)(m_state | STATE_ACTIVE);
    m_state = (STATE)(m_state ^ STATE_STARTING);

    DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Stop(IComponent* listener)
{
    DebugPrintLn(TEXT("%s::Stop() begin"), COMP_NAME);

    if ( !(m_state & STATE_ACTIVE) )
    {
        DebugPrintLn(TEXT("Already stopped"));
        return S_FALSE;
    }
    if ( m_state & STATE_STOPPING )
    {
        DebugPrintLn(TEXT("Now stopping"));
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STOPPING);

    // 子コンポーネントの破棄
    if ( pimpl->child )
    {
        pimpl->child->Release();
        pimpl->child = nullptr;
    }

    // メインウィンドウの破棄
    #if defined(_DEBUG) || defined(DEBUG)
    if ( pimpl->mwnd )
    {
        delete pimpl->mwnd;
        pimpl->mwnd = nullptr;
    }
    #endif

    m_state = (STATE)(m_state ^ STATE_ACTIVE);
    m_state = (STATE)(m_state ^ STATE_STOPPING);

    DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CubeMelon.Host.cpp