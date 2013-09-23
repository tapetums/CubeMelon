// OutputComponentBase.cpp

#pragma once

//---------------------------------------------------------------------------//

#include <map>
#include <string>

#include <windows.h>

#include "ComPtr.h"
#include "DebugPrint.h"
#include "LockModule.h"
#include "Interfaces.h"
#include "ComponentBase.h"

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const wchar_t* COMP_BASE = TEXT("OutputComponentBase");

//---------------------------------------------------------------------------//

OutputComponentBase::OutputComponentBase(IUnknown* pUnkOuter)
{
    DebugPrintLn(TEXT("%s::%s::Constructor() begin"), COMP_NAME, COMP_BASE);

    m_notify_map = new NotifyMap;

    m_cRef     = 0;
    m_state    = STATE_IDLE;
    m_owner    = nullptr;
    m_prop_mgr = nullptr;
    m_position = 0;
    m_size     = 0;
    m_path[0]  = '\0';

    if ( pUnkOuter )
    {
        DebugPrintLn(TEXT(". Getting owner's interface..."));
        auto hr = pUnkOuter->QueryInterface
        (
            IID_IComponent, (void**)&m_owner
        );
        DebugPrintLn(TEXT(". Got owner's interface"));
    }

    this->AddRef();

    DebugPrintLn(TEXT("%s::%s::Constructor() end"), COMP_NAME, COMP_BASE);
}

//---------------------------------------------------------------------------//

OutputComponentBase::~OutputComponentBase()
{
    DebugPrintLn(TEXT("%s::%s::Destructor() begin"), COMP_NAME, COMP_BASE);

    m_state = STATE_TERMINATING;

    if ( m_prop_mgr )
    {
        m_prop_mgr->Release();
        m_prop_mgr = nullptr;
    }
    if ( m_owner )
    {
        DebugPrintLn(TEXT(". Releasing %s's Owner..."), COMP_NAME);
        {
            m_owner->Release();
            m_owner = nullptr;
        }
        DebugPrintLn(TEXT(". Released %s's Owner"), COMP_NAME);
    }

    m_notify_map->clear();
    delete m_notify_map;
    m_notify_map = nullptr;

    m_state = STATE_IDLE;

    DebugPrintLn(TEXT("%s::%s::Destructor() end"), COMP_NAME, COMP_BASE);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    DebugPrintLn(TEXT("%s::%s::QueryInterface() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IComponent) )
    {
        DebugPrintLn(TEXT(". IID_IComponent"));
        *ppvObject = static_cast<IComponent*>(this);
    }
    else if ( IsEqualIID(riid, IID_IIOComponent) )
    {
        DebugPrintLn(TEXT(". IID_IIOComponent"));
        *ppvObject = static_cast<IIOComponent*>(this);
    }
    else if ( IsEqualIID(riid, IID_IOutputComponent) )
    {
        DebugPrintLn(TEXT(". IID_IOutputComponent"));
        *ppvObject = static_cast<IOutputComponent*>(this);
    }
    else
    {
        DebugPrintLn(TEXT(". No such an interface"));
        DebugPrintLn(TEXT("%s::%s::QueryInterface() end"), COMP_NAME, COMP_BASE);
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(TEXT("%s::%s::QueryInterface() end"), COMP_NAME, COMP_BASE);

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall OutputComponentBase::AddRef()
{
    DebugPrintLn(TEXT("%s::%s::AddRef() begin %d"), COMP_NAME, COMP_BASE, m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(TEXT("%s::%s::AddRef() end %d"), COMP_NAME, COMP_BASE, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall OutputComponentBase::Release()
{
    DebugPrintLn(TEXT("%s::%s::Release() begin %d"), COMP_NAME, COMP_BASE, m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT(". Deleting..."));
        delete this;
        DebugPrintLn(TEXT(". Deleted"));
    }

    UnlockModule();

    DebugPrintLn(TEXT("%s::%s::Release() end %d"), COMP_NAME, COMP_BASE, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall OutputComponentBase::ClassID() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall OutputComponentBase::Name() const
{
    return COMP_NAME;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall OutputComponentBase::Owner() const
{
    if ( m_owner )
    {
        m_owner->AddRef();
    }

    return m_owner;
}

//---------------------------------------------------------------------------//

IPropManager* __stdcall OutputComponentBase::PropManager() const
{
    if ( m_prop_mgr )
    {
        m_prop_mgr->AddRef();
    }

    return m_prop_mgr;
}

//---------------------------------------------------------------------------//

STATE __stdcall OutputComponentBase::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Attach
(
    LPCWSTR msg, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Attach() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    DebugPrintLn(TEXT(". MSG: %s, Listener: %s"), msg, listener->Name());

    auto range = m_notify_map->equal_range(std::wstring(msg));
    auto it  = range.first;
    auto itE = range.second;
    while ( it != itE )
    {
        if ( it->second == listener )
        {
            DebugPrintLn(TEXT(". . This message has been already attached to the component"));
            DebugPrintLn(TEXT("%s::%s::Attach() end"), COMP_NAME, COMP_BASE);
            return S_FALSE;
        }
        ++it;
    }

    m_notify_map->emplace
    (
        std::make_pair(std::wstring(msg), ComPtr<IComponent>(listener))
    );

    DebugPrintLn(TEXT("%s::%s::Attach() end"), COMP_NAME, COMP_BASE);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Detach
(
    LPCWSTR msg, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Detach() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    DebugPrintLn(TEXT(". MSG: %s, Listener: %s"), msg, listener->Name());

    size_t count = 0;
    auto range = m_notify_map->equal_range(std::wstring(msg));
    auto it  = range.first;
    auto itE = range.second;

     while ( it != itE )
    {
        if ( it->second == listener )
        {
            m_notify_map->erase(it);
            ++count;
        }
        ++it;
    }
    if ( count == 0 )
    {
        DebugPrintLn(TEXT(". . This message was not attached to the component"));
        DebugPrintLn(TEXT("%s::%s::Detach() end"), COMP_NAME, COMP_BASE);
        return S_FALSE;
    }

    DebugPrintLn(TEXT("%s::%s::Detach() end"), COMP_NAME, COMP_BASE);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::GetInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
{
    DebugPrintLn(TEXT("%s::%s::GetInstance() begin"), COMP_NAME, COMP_BASE);

    HRESULT hr;

    if ( IsEqualCLSID(rclsid, CLSID_Component) )
    {
        hr = this->QueryInterface(riid, ppvObject);
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

    DebugPrintLn(TEXT("%s::%s::GetInstance() end"), COMP_NAME, COMP_BASE);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Notify
(
    IMsgObject* msg_obj
)
{
    DebugPrintLn(TEXT("%s::%s::Notify() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == msg_obj )
    {
        DebugPrintLn(TEXT(". Message object is void"));
        return E_POINTER;
    }

    msg_obj->Release();
    msg_obj = nullptr;

    DebugPrintLn(TEXT("%s::%s::Notify() end"), COMP_NAME, COMP_BASE);

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Start
(
    LPVOID args, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Start() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Start() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Stop
(
    IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Stop() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Stop() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Close
(
    IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Close() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Close() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Open
(
    LPCWSTR path, LPCWSTR format_as, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Open() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Open() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::QuerySupport
(
    LPCWSTR path, LPCWSTR format_as
)
{
    DebugPrintLn(TEXT("%s::%s::QuerySupport() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::QuerySupport() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Seek
(
    INT64 offset, DWORD origin, UINT64* new_pos
)
{
    DebugPrintLn(TEXT("%s::%s::Seek() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Seek() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall OutputComponentBase::Write
(
    LPVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Write() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Write() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// OutputComponentBase.cpp