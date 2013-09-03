﻿// CubeMelon.Host.cpp

#include <memory>
#include <map>

#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "PluginManager.h"
#include "Host.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Host")

extern const CLSID CLSID_Plugin;
extern HINSTANCE g_hInst;

//---------------------------------------------------------------------------//

struct Host::Impl
{
    Impl();
    ~Impl();

    ::PluginManager* manager;
    IPlugin*         child;
};

//---------------------------------------------------------------------------//

Host::Impl::Impl()
{
    manager = nullptr;
    child  = nullptr;
}

//---------------------------------------------------------------------------//

Host::Impl::~Impl()
{
    if ( child )
    {
        child->Release();
        child = nullptr;
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
    ::PathAppend(dir_path, TEXT("\\plugins"));
    pimpl->manager = new ::PluginManager(dir_path);

    m_cRef  = 0;
    m_owner = nullptr;
    m_state = STATE_IDLE;

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

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPlugin) )
    {
        *ppvObject = static_cast<IPlugin*>(this);
    }
    else if ( IsEqualIID(riid, IID_IPluginHost) )
    {
        *ppvObject = static_cast<IPluginHost*>(this);
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
    return CLSID_Plugin;
}

//---------------------------------------------------------------------------//

IPlugin* __stdcall Host::Owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

STATE __stdcall Host::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Attach(LPCWSTR msg, IPlugin* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Detach(LPCWSTR msg, IPlugin* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::Notify
(
    IPlugin* sender, LPCWSTR msg, LPVOID data, size_t cb_data
)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Host::GetPluginInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
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

    size_t index = 0;
    auto count = pimpl->manager->PluginCount();
    IPluginContainer* pc = nullptr;
    for ( ; index < count; ++index )
    {
        pc = pimpl->manager->PluginContainer(index);
        if ( pc && IsEqualCLSID(rclsid, pc->ClassID()) )
        {
            break;
        }
    }
    if ( index == count )
    {
        return CS_E_CLASS_NOTFOUND;
    }

    IClassFactory* factory = nullptr;
    hr = pc->GetClassObject
    (
        IID_IClassFactory, (void**)&factory
    );
    if ( FAILED(hr) || nullptr == factory )
    {
        return hr;
    }

    hr = factory->CreateInstance(static_cast<IUnknown*>((void*)this), riid, ppvObject);
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

    // プラグインの読み込み
    DebugPrintLn(TEXT("Loading plugins..."));
    {
        hr = pimpl->manager->LoadAll();
        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    DebugPrintLn(TEXT("Loaded plugins"));

    // プラグインの起動
    DebugPrintLn(TEXT("Executing plugin..."));
    {
        CLSID clsid =
        { 0x520e13ad, 0x3345, 0x4377, { 0xb2, 0xef, 0x68, 0x42, 0xea, 0x79, 0xb2, 0x5b } };

        hr = this->GetPluginInstance
        (
            clsid, IID_IPlugin, (void**)&pimpl->child
        );
        if ( FAILED(hr) || nullptr == pimpl->child )
        {
            DebugPrintLn(TEXT("Plugin was not found"));
            return hr;
        }

        hr = pimpl->child->Start(nullptr);
    }
    DebugPrintLn(TEXT("Executed plugin"));

    // 起動させなかったプラグインは一旦アンロード
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

    if ( pimpl->child )
    {
        pimpl->child->Release();
        pimpl->child = nullptr;
    }

    DebugPrintLn(NAME TEXT("::Stop() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

IPluginManager* __stdcall Host::PluginManager() const
{
    return pimpl->manager;
}

//---------------------------------------------------------------------------//

// CubeMelon.Host.cpp