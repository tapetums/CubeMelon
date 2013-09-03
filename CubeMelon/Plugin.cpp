// CubeMelon.Plugin.cpp

#include <memory>
#include <map>

#include <windows.h>
#include <propsys.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\PluginManager.h"
#include "..\include\Plugin.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Host")

extern const CLSID CLSID_Plugin;
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//

struct Plugin::Impl
{
    Impl();
    ~Impl();

    ::PluginManager manager;
    Plugin* plugin;
};

//---------------------------------------------------------------------------//

Plugin::Impl::Impl()
{
    plugin = nullptr;
}

//---------------------------------------------------------------------------//

Plugin::Impl::~Impl()
{
    if ( plugin )
    {
        plugin->Release();
        plugin = nullptr;
    }
}

//---------------------------------------------------------------------------//

Plugin::Plugin(IUnknown* pUnkOuter)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    pimpl = new Impl;

    m_cRef  = 0;
    m_owner = nullptr;
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
    m_owner = nullptr;
    m_cRef  = 0;

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
        DebugPrintLn(TEXT("\tDeleting..."));
        delete this;
        DebugPrintLn(TEXT("\tDeleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

::PluginManager* __stdcall Plugin::PluginManager() const
{
    return &(pimpl->manager);
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
    DebugPrintLn(NAME TEXT("::Status() %d"), m_state);
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
    if ( 0 == lstrcmp(msg, TEXT("GetPluginManager")) )
    {
        if ( nullptr == data )
        {
            return E_INVALIDARG;
        }
        auto pm = (::PluginManager**)data;
        *pm = &(pimpl->manager);

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::GetPluginInstance(REFCLSID rclsid, REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME TEXT("::GetPluginInstance() begin"));

    HRESULT hr;

    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if ( IsEqualCLSID(rclsid, CLSID_Plugin) )
    {
        return this->QueryInterface(riid, ppvObject);
    }

    size_t i = 0;
    auto count = pimpl->manager.PluginCount();
    auto plugins = pimpl->manager.AllPlugins();
    PluginInstance* pi = nullptr;
    for ( ; i < count; ++i )
    {
        pi = plugins[i];
        if ( pi && IsEqualCLSID(rclsid, pi->ClassID()) )
        {
            break;
        }
    }
    if ( nullptr == pi || i == count )
    {
        return CS_E_CLASS_NOTFOUND;
    }

    IClassFactory* factory = nullptr;
    hr = pi->GetClassObject
    (
        IID_IClassFactory, (void**)&factory
    );
    if ( FAILED(hr) || nullptr == factory )
    {
        return hr;
    }

    hr = factory->CreateInstance(this, riid, ppvObject);
    factory->Release();
    factory = nullptr;

    if ( FAILED(hr) || nullptr == *ppvObject )
    {
        return hr;
    }

    /// 何の処理を入れるか未定 ///

    DebugPrintLn(NAME TEXT("::GetPluginInstance() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Start(LPCVOID args)
{
    DebugPrintLn(NAME TEXT("::Start() begin"));

    HRESULT hr = S_FALSE;

    if ( m_state == STATE_RUNNING )
    {
        DebugPrintLn(TEXT("\tAlready started"));

        return hr;
    }

    m_state = STATE_RUNNING;

    // プラグインの読み込み
    DebugPrintLn(TEXT("\tLoading plugins..."));
    {
        hr = pimpl->manager.LoadAll();
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    DebugPrintLn(TEXT("\tLoaded plugins"));

    // プラグインの起動
    DebugPrintLn(TEXT("\tExecuting plugin..."));
    {
        CLSID clsid =
        { 0x520e13ad, 0x3345, 0x4377, { 0xb2, 0xef, 0x68, 0x42, 0xea, 0x79, 0xb2, 0x5b } };

        hr = this->GetPluginInstance
        (
            clsid, IID_IPlugin, (void**)&pimpl->plugin
        );
        if ( FAILED(hr) || nullptr == pimpl->plugin )
        {
            DebugPrintLn(TEXT("\tPlugin was not found"));
            return hr;
        }

        hr = pimpl->plugin->Start(nullptr);
    }
    DebugPrintLn(TEXT("\tExecuted plugin"));

    // 起動させなかったプラグインは一旦アンロード
    pimpl->manager.FreeAll();

    DebugPrintLn(NAME TEXT("::Start() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Plugin::Stop()
{
    DebugPrintLn(NAME TEXT("::Stop() begin"));

    if ( m_state == STATE_IDLE )
    {
        DebugPrintLn(TEXT("\tAlready stopped"));
        return S_FALSE;
    }

    m_state = STATE_IDLE;

    if ( pimpl->plugin )
    {
        pimpl->plugin->Release();
        pimpl->plugin = nullptr;
    }

    DebugPrintLn(NAME TEXT("::Stop() end"));

    return S_OK;
}

// CubeMelon.Plugin.cpp