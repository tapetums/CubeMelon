// Output.Wasapi.Wasapi.cpp

#include <windows.h>
#include <strsafe.h>
#include <mmreg.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "Wasapi.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Output.Wasapi")

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct Wasapi::Impl
{
    Impl();
    ~Impl();

    bool    IsSupportedFormat(LPCWSTR format);
    HRESULT Close();
    HRESULT Open(LPCWSTR path);

    UINT64               size;
    WAVEFORMATEXTENSIBLE wfex;
    WCHAR                path[MAX_PATH];
};

//---------------------------------------------------------------------------//

Wasapi::Impl::Impl()
{
    size      = 0;
    path[0]   = '\0';
    ::ZeroMemory(&wfex, sizeof(WAVEFORMATEXTENSIBLE));
}

//---------------------------------------------------------------------------//

Wasapi::Impl::~Impl()
{
    this->Close();
}

//---------------------------------------------------------------------------//

bool Wasapi::Impl::IsSupportedFormat(LPCWSTR format)
{
    if (format[0] == '.' )
    {
        ++format;
    }

    if ( lstrcmp(format, TEXT("wav"))  == 0 ||
         lstrcmp(format, TEXT("WAV"))  == 0 ||
         lstrcmp(format, TEXT("riff")) == 0 ||
         lstrcmp(format, TEXT("RIFF")) == 0 ||
         lstrcmp(format, TEXT("rf64")) == 0 ||
         lstrcmp(format, TEXT("RF64")) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------//

HRESULT Wasapi::Impl::Close()
{
    DebugPrintLn(NAME TEXT("Impl::Close() begin"));

    DebugPrintLn(NAME TEXT("Impl::Close() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT Wasapi::Impl::Open(LPCWSTR path)
{
    DebugPrintLn(NAME TEXT("Impl::Open() begin"));
    DebugPrintLn(path);

    // パスを内部領域にコピー
    ::StringCchCopy(this->path, MAX_PATH, path);

    try
    {
    }
    catch (LPCWSTR msg)
    {
        DebugPrintLn(msg);

        this->Close();

        return E_FAIL;
    }

    DebugPrintLn(NAME TEXT("Impl::Open() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

Wasapi::Wasapi(IUnknown* pUnkOuter)
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

Wasapi::~Wasapi()
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

HRESULT __stdcall Wasapi::QueryInterface(REFIID riid, void** ppvObject)
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
    else if ( IsEqualIID(riid, IID_IIOComponent) )
    {
        *ppvObject = static_cast<IIOComponent*>(this);
    }
    else if ( IsEqualIID(riid, IID_IOutputComponent) )
    {
        *ppvObject = static_cast<IOutputComponent*>(this);
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

ULONG __stdcall Wasapi::AddRef()
{
    DebugPrintLn(NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Wasapi::Release()
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

REFCLSID __stdcall Wasapi::ClassID() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall Wasapi::Owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IPropertyStore* __stdcall Wasapi::Property() const
{
    return nullptr;
}

//---------------------------------------------------------------------------//

STATE __stdcall Wasapi::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Attach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Detach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::GetInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
{
    if ( m_owner )
    {
        return m_owner->GetInstance(rclsid, riid, ppvObject);
    }
    else
    {
        return E_NOTIMPL;
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Notify
(
    IComponent* sender, LPCWSTR msg, LPVOID data, size_t cb_data
)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Start(LPCVOID args)
{
    DebugPrintLn(NAME TEXT("::Start() begin"));

    HRESULT hr;

    if ( m_state == STATE_RUNNING )
    {
        DebugPrintLn(TEXT("Already started"));
        return S_FALSE;
    }

    /// ここに処理を書く

    m_state = STATE_RUNNING;

    DebugPrintLn(NAME TEXT("::Start() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Stop()
{
    DebugPrintLn(NAME TEXT("::Stop() begin"));

    if ( m_state == STATE_IDLE )
    {
        DebugPrintLn(TEXT("Already stopped"));
        return S_FALSE;
    }

    this->Close();

    m_state = STATE_IDLE;

    DebugPrintLn(NAME TEXT("::Stop() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Close(IComponent* listener)
{
    DebugPrintLn(NAME TEXT("::Close() begin"));

    if ( !(m_state & STATE_OPEN) )
    {
        return S_FALSE;
    }

    HRESULT hr;

    if ( listener )
    {
        //hr = pimpl->CloseAsync(pimpl, listener);
        hr = E_NOTIMPL;
    }
    else
    {
        hr = pimpl->Close();
        if ( SUCCEEDED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_OPEN);
        }
    }

    DebugPrintLn(NAME TEXT("::Close() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Open
(
    LPCWSTR path, LPCWSTR format_as, IComponent* listener
)
{
    DebugPrintLn(NAME TEXT("::Open() begin"));

    if ( !pimpl->IsSupportedFormat(format_as) )
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    if ( listener )
    {
        //hr = pimpl->OpenAsync(pimpl, path, listener);
        hr = E_NOTIMPL;
    }
    else
    {
        hr = pimpl->Open(path);
        if ( SUCCEEDED(hr) )
        {
            m_state = (STATE)(m_state | STATE_OPEN);
        }
    }

    DebugPrintLn(NAME TEXT("::Open() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::QuerySupport(LPCWSTR path, LPCWSTR format_as)
{
    DebugPrintLn(NAME TEXT("::QuerySupport() begin"));

    if ( !pimpl->IsSupportedFormat(format_as) )
    {
        return E_FAIL;
    }

    DebugPrintLn(NAME TEXT("::QuerySupport() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Seek(INT64 offset, DWORD origin, UINT64* new_pos)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wasapi::Write
(
    LPCVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener
)
{
    DebugPrintLn(NAME TEXT("::Write() begin"));

    DebugPrintLn(NAME TEXT("::Write() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Output.Wasapi.Wasapi.cpp