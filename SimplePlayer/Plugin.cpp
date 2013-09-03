// SimplePlayer.Plugin.cpp

#include <windows.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\Plugin.h"

#include "mainwindow.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("SimplePlayer")

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Plugin;

//---------------------------------------------------------------------------//

struct Plugin::Impl
{
    Impl();
    ~Impl();

    MainWindow* mwnd;
};

//---------------------------------------------------------------------------//

Plugin::Impl::Impl()
{
    mwnd = nullptr;
}

//---------------------------------------------------------------------------//

Plugin::Impl::~Impl()
{
    if ( mwnd )
    {
        delete mwnd;
        mwnd = nullptr;
    }
}

//---------------------------------------------------------------------------//

Plugin::Plugin(IUnknown* pUnkOuter)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    pimpl = new Impl;

    m_cRef  = 0;
    m_owner = nullptr;
    if ( pUnkOuter )
    {
        auto hr = pUnkOuter->QueryInterface
        (
            IID_IPlugin, (void**)&m_owner
        );
        if ( FAILED(hr) )
        {
            m_owner = nullptr;
        }
    }
    m_state = STATE_IDLE;

    this->AddRef();

    DebugPrintLn(NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

Plugin::~Plugin()
{
    DebugPrintLn(NAME TEXT("::Destructor() begin"));

    this->Stop();

    m_state = STATE_TERMINATING;
    if ( m_owner )
    {
        DebugPrintLn(TEXT("Releasing ") NAME TEXT("'s Owner..."));

        m_owner->Release();
        m_owner = nullptr;

        DebugPrintLn(TEXT("Released ") NAME TEXT("'s Owner"));
    }
    m_cRef = 0;

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPlugin) )
    {
        *ppvObject = static_cast<IPlugin*>(this);
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

ULONG __stdcall Plugin::AddRef()
{
    DebugPrintLn(NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Plugin::Release()
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

::PluginManager* __stdcall Plugin::PluginManager() const
{
    return m_owner ? m_owner->PluginManager() : nullptr;
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall Plugin::ClassID() const
{
    return CLSID_Plugin;
}

//---------------------------------------------------------------------------//

IPlugin* __stdcall Plugin::Owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

STATE __stdcall Plugin::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Attach(LPCWSTR msg, IPlugin* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Detach(LPCWSTR msg, IPlugin* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Notify
(
    IPlugin* sender, LPCWSTR msg, LPVOID data, size_t cb_data
)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::GetPluginInstance(REFCLSID rclsid, REFIID riid, void** ppvObject)
{
    if ( m_owner )
    {
        return m_owner->GetPluginInstance(rclsid, riid, ppvObject);
    }
    else
    {
        return E_NOTIMPL;
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Start(LPCVOID args)
{
    DebugPrintLn(NAME TEXT("::Start() begin"));

    HRESULT hr = S_FALSE;

    if ( m_state == STATE_RUNNING )
    {
        DebugPrintLn(TEXT("Already started"));

        return S_FALSE;
    }

    /// ここに処理を書く
    pimpl->mwnd = new MainWindow;
    pimpl->mwnd->setOwner(this);
    pimpl->mwnd->show();

    m_state = STATE_RUNNING;

    DebugPrintLn(NAME TEXT("::Start() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Stop()
{
    DebugPrintLn(NAME TEXT("::Stop() begin"));

    if ( m_state == STATE_IDLE )
    {
        DebugPrintLn(TEXT("Already stopped"));
        return S_FALSE;
    }

    m_state = STATE_IDLE;

    if ( pimpl->mwnd )
    {
        delete pimpl->mwnd;
        pimpl->mwnd = nullptr;
    }
    if ( m_owner )
    {
        auto hr = m_owner->Stop();
        if ( S_OK != hr )
        {
            DebugPrintLn(TEXT("Failed to Stop ") NAME TEXT("'s Owner"));
        }
    }

    DebugPrintLn(NAME TEXT("::Stop() end"));

    return S_OK;
}

// SimplePlayer.Plugin.cpp