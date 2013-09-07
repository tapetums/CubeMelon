// UI.SimplePlayer.SimplePlayer.cpp

#include <windows.h>

#include <propsys.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "SimplePlayer.h"

#include "mainwindow.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("UI.SimplePlayer")

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component;

STDAPI DllGetProperty(size_t index, IPropertyStore** ps);

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct SimplePlayer::Impl
{
    Impl();
    ~Impl();

    MainWindow* mwnd;
};

//---------------------------------------------------------------------------//

SimplePlayer::Impl::Impl()
{
    mwnd = nullptr;
}

//---------------------------------------------------------------------------//

SimplePlayer::Impl::~Impl()
{
    if ( mwnd )
    {
        delete mwnd;
        mwnd = nullptr;
    }
}

//---------------------------------------------------------------------------//

SimplePlayer::SimplePlayer(IUnknown* pUnkOuter)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    pimpl = new Impl;

    m_cRef  = 0;
    m_state = STATE_IDLE;
    m_owner = nullptr;
    if ( pUnkOuter )
    {
        auto hr = pUnkOuter->QueryInterface
        (
            IID_IComponent, (void**)&m_owner
        );
        if ( FAILED(hr) )
        {
            m_owner = nullptr;
        }
    }

    this->AddRef();

    DebugPrintLn(NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

SimplePlayer::~SimplePlayer()
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

HRESULT __stdcall SimplePlayer::QueryInterface(REFIID riid, void** ppvObject)
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
    else if ( IsEqualIID(riid, IID_IUIComponent) )
    {
        *ppvObject = static_cast<IUIComponent*>(this);
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

ULONG __stdcall SimplePlayer::AddRef()
{
    DebugPrintLn(NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall SimplePlayer::Release()
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

REFCLSID __stdcall SimplePlayer::ClassID() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall SimplePlayer::Owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IPropertyStore* __stdcall SimplePlayer::Property() const
{
    return nullptr;
}

//---------------------------------------------------------------------------//

STATE __stdcall SimplePlayer::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::Attach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::Detach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::GetInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
{
    HRESULT hr;

    if ( IsEqualCLSID(rclsid, CLSID_Component) )
    {
        if ( IsEqualIID(riid, IID_IPropertyStore))
        {
            hr = DllGetProperty(0, (IPropertyStore**)ppvObject);
        }
        else
        {
            hr = this->QueryInterface(riid, ppvObject);
        }
    }
    else
    {
        if ( m_owner )
        {
            hr = m_owner->GetInstance(rclsid, riid, ppvObject);
        }
        else
        {
            hr = E_NOTIMPL;
        }
    }

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::Notify
(
    IComponent* sender, LPCWSTR msg, LPVOID data, size_t cb_data
)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::Start(LPCVOID args)
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

HRESULT __stdcall SimplePlayer::Stop()
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

//---------------------------------------------------------------------------//

size_t __stdcall SimplePlayer::WindowCount() const
{
    return pimpl->mwnd ? 1 : 0;
}

//---------------------------------------------------------------------------//

HWND   __stdcall SimplePlayer::Handle(size_t index) const
{
    if ( index > 1 || nullptr == pimpl->mwnd )
    {
        return nullptr;
    }
    else
    {
        return reinterpret_cast<HWND>(pimpl->mwnd->winId());
    }
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

// UI.SimplePlayer.SimplePlayer.cpp