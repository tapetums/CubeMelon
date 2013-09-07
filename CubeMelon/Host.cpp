// CubeMelon.Host.cpp

#include <memory>
#include <map>

#include <windows.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <propsys.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\PropertyStore.h"
#include "..\include\Interfaces.h"

#include "ComponentManager.h"
#include "Host.h"

#include "mainwindow.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Host")

extern const CLSID CLSID_Component;
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct Host::Impl
{
    Impl();
    ~Impl();

    ComponentManager* manager;
    IComponent*       child;
    MainWindow*       mwnd;
    CLSID             clsid_autorun;
};

//---------------------------------------------------------------------------//

Host::Impl::Impl()
{
    manager = nullptr;
    child   = nullptr;
    mwnd    = nullptr;

    CLSID clsid =
    { 0x520e13ad, 0x3345, 0x4377, { 0xb2, 0xef, 0x68, 0x42, 0xea, 0x79, 0xb2, 0x5b } };
    clsid_autorun = clsid;//CLSID_NULL;
}

//---------------------------------------------------------------------------//

Host::Impl::~Impl()
{
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
}

//---------------------------------------------------------------------------//

Host::Host(IUnknown* pUnkOuter)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    pimpl = new Impl;

    WCHAR dir_path[MAX_PATH];
    ::GetModuleFileName(nullptr, dir_path, MAX_PATH);
    ::PathRemoveFileSpec(dir_path);
    ::PathAppend(dir_path, TEXT("\\components"));
    pimpl->manager = new ComponentManager(dir_path);

    m_cRef  = 0;
    m_state = STATE_IDLE;
    m_owner = nullptr;

    this->AddRef();

    DebugPrintLn(NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

Host::~Host()
{
    DebugPrintLn(NAME TEXT("::Destructor() begin"));

    this->Stop();

    m_state = STATE_TERMINATING;
    m_owner = nullptr;
    m_cRef  = 0;

    if ( pimpl->manager )
    {
        pimpl->manager->Release();
        pimpl->manager = nullptr;
    }

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IComponent) )
    {
        *ppvObject = static_cast<IComponent*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(NAME TEXT("::QueryInterface() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall Host::AddRef()
{
    DebugPrintLn(NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Host::Release()
{
    DebugPrintLn(NAME TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall Host::ClassID() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall Host::Owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IPropertyStore* __stdcall Host::Property() const
{
    return nullptr;
}

//---------------------------------------------------------------------------//

STATE __stdcall Host::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Attach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Detach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::GetInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
{
    DebugPrintLn(NAME TEXT("::GetInstance() begin"));

    HRESULT hr;

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualCLSID(rclsid, CLSID_Component) )
    {
        if ( IsEqualIID(riid, IID_IPropertyStore))
        {
            static PropertyStore ps;
            *ppvObject = &ps;
            ps.AddRef();
            hr = S_OK;
        }
        else if ( IsEqualIID(riid, IID_IComponentManager) )
        {
            *ppvObject = pimpl->manager;
            pimpl->manager->AddRef();
            hr = S_OK;
        }
        else
        {
            hr = this->QueryInterface(riid, ppvObject);
        }
    }
    else if ( pimpl->child && IsEqualCLSID(rclsid, pimpl->child->ClassID()) )
    {
        hr = pimpl->child->QueryInterface(riid, ppvObject);
    }
    else
    {
        auto cc = pimpl->manager->Find(rclsid);
        if ( nullptr == cc )
        {
            hr = CS_E_CLASS_NOTFOUND;
        }
        else
        {
            hr = cc->CreateInstance(this, riid, ppvObject);
        }
        cc->Release();
    }

    DebugPrintLn(NAME TEXT("::GetInstance() end"));
    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Notify
(
    IComponent* sender, LPCWSTR msg, LPVOID data, size_t cb_data
)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Start(LPCVOID args)
{
    DebugPrintLn(NAME TEXT("::Start() begin"));

    HRESULT hr = S_FALSE;

    if ( m_state == STATE_RUNNING )
    {
        DebugPrintLn(TEXT("Already started"));

        return hr;
    }

    m_state = STATE_RUNNING;

    // コンポーネントの読み込み
    DebugPrintLn(TEXT("Loading components..."));
    {
        hr = pimpl->manager->LoadAll();
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    DebugPrintLn(TEXT("Loaded components"));

#if defined(_DEBUG) || defined(DEBUG)
    // メインウィンドウの起動
    pimpl->mwnd = new MainWindow;
    if ( pimpl->mwnd )
    {
        IComponentContainer* cc = nullptr;
        auto count = pimpl->manager->ComponentCount();
        for ( size_t index = 0; index < count; ++index )
        {
            cc = pimpl->manager->At(index);
            if ( cc )
            {
                pimpl->mwnd->addListItem(cc);
                pimpl->mwnd->addConsoleText(cc->Name());
            }
        }
        pimpl->mwnd->addConsoleText(TEXT("Ready"));
        pimpl->mwnd->show();
    }
#endif

    // 子コンポーネントの起動
    DebugPrintLn(TEXT("Executing component..."));
    {
        hr = this->GetInstance
        (
            pimpl->clsid_autorun, IID_IComponent, (void**)&pimpl->child
        );
        if ( FAILED(hr) || nullptr == pimpl->child )
        {
            DebugPrintLn(TEXT("Component was not found"));
            return hr;
        }

        hr = pimpl->child->Start(nullptr);
    }
    DebugPrintLn(TEXT("Executed component"));

    // 起動させなかったコンポーネントは一旦アンロード
    pimpl->manager->FreeAll();

    DebugPrintLn(NAME TEXT("::Start() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Stop()
{
    DebugPrintLn(NAME TEXT("::Stop() begin"));

    if ( m_state == STATE_IDLE )
    {
        DebugPrintLn(TEXT("Already stopped"));
        return S_FALSE;
    }

    m_state = STATE_IDLE;

    // 子コンポーネントの破棄
    if ( pimpl->child )
    {
        pimpl->child->Release();
        pimpl->child = nullptr;
    }

#if defined(_DEBUG) || defined(DEBUG)
    // メインウィンドウの破棄
    if ( pimpl->mwnd )
    {
        delete pimpl->mwnd;
        pimpl->mwnd = nullptr;
    }

    DebugPrintLn(NAME TEXT("::Stop() end"));
#endif

    return S_OK;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CubeMelon.Host.cpp