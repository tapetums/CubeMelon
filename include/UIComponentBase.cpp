// UIComponentBase.cpp

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

extern const wchar_t* COMP_BASE = TEXT("UIComponentBase");

//---------------------------------------------------------------------------//

UIComponentBase::UIComponentBase(IUnknown* pUnkOuter)
{
    DebugPrintLn(TEXT("%s::%s::Constructor() begin"), COMP_NAME, COMP_BASE);

    m_notify_map = new NotifyMap;

    m_cRef  = 0;
    m_state = STATE_IDLE;
    m_owner = nullptr;

    DllGetPropManager(COMP_INDEX, &m_prop_mgr);

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

    DebugPrintLn(TEXT("%s::%s::Constructor() end"), COMP_NAME, COMP_BASE);
}

//---------------------------------------------------------------------------//

UIComponentBase::~UIComponentBase()
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
        DebugPrintLn(TEXT("Releasing %s's Owner..."), COMP_NAME);

        m_owner->Release();
        m_owner = nullptr;

        DebugPrintLn(TEXT("Released %s's Owner"), COMP_NAME);
    }

    m_notify_map->clear();
    delete m_notify_map;
    m_notify_map = nullptr;

    m_state = STATE_IDLE;

    DebugPrintLn(TEXT("%s::%s::Destructor() end"), COMP_NAME, COMP_BASE);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UIComponentBase::QueryInterface
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

    DebugPrintLn(TEXT("%s::%s::QueryInterface() end"), COMP_NAME, COMP_BASE);

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall UIComponentBase::AddRef()
{
    DebugPrintLn(TEXT("%s::%s::AddRef() begin %d"), COMP_NAME, COMP_BASE, m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(TEXT("%s::%s::AddRef() end %d"), COMP_NAME, COMP_BASE, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall UIComponentBase::Release()
{
    DebugPrintLn(TEXT("%s::%s::Release() begin %d"), COMP_NAME, COMP_BASE, m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(TEXT("%s::%s::Release() end %d"), COMP_NAME, COMP_BASE, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall UIComponentBase::ClassID() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall UIComponentBase::Name() const
{
    return COMP_NAME;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall UIComponentBase::Owner() const
{
    if ( m_owner )
    {
        m_owner->AddRef();
    }

    return m_owner;
}

//---------------------------------------------------------------------------//

IPropManager* __stdcall UIComponentBase::PropManager() const
{
    if ( m_prop_mgr )
    {
        m_prop_mgr->AddRef();
    }

    return m_prop_mgr;
}

//---------------------------------------------------------------------------//

STATE __stdcall UIComponentBase::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UIComponentBase::Attach
(
    LPCWSTR msg, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Attach() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    DebugPrintLn(TEXT("MSG: %s, Listener: %s"), msg, listener->Name());

    auto range = m_notify_map->equal_range(std::wstring(msg));
    auto it  = range.first;
    auto itE = range.second;
    while ( it != itE )
    {
        if ( it->second == listener )
        {
            DebugPrintLn(TEXT("This message has been already attached to the component"));
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

HRESULT __stdcall UIComponentBase::Detach
(
    LPCWSTR msg, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Detach() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    DebugPrintLn(TEXT("MSG: %s, Listener: %s"), msg, listener->Name());

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
        DebugPrintLn(TEXT("This message was not attached to the component"));
        return S_FALSE;
    }

    DebugPrintLn(TEXT("%s::%s::Detach() end"), COMP_NAME, COMP_BASE);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UIComponentBase::GetInstance
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

HRESULT __stdcall UIComponentBase::Notify
(
    IMsgObject* msg_obj
)
{
    DebugPrintLn(TEXT("%s::%s::Notify() begin"), COMP_NAME, COMP_BASE);

    if ( nullptr == msg_obj )
    {
        DebugPrintLn(TEXT("Message object is void"));
        return E_POINTER;
    }

    msg_obj->Release();
    msg_obj = nullptr;

    DebugPrintLn(TEXT("%s::%s::Notify() end"), COMP_NAME, COMP_BASE);

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UIComponentBase::Start
(
    LPVOID args, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Start() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Start() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall UIComponentBase::Stop
(
    IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::%s::Stop() begin"), COMP_NAME, COMP_BASE);
    DebugPrintLn(TEXT("%s::%s::Stop() end"), COMP_NAME, COMP_BASE);
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

size_t __stdcall UIComponentBase::WindowCount() const
{
    return m_hwnd ? 1 : 0;
}

//---------------------------------------------------------------------------//

HWND __stdcall UIComponentBase::Handle(size_t index) const
{
    return ( index == 0 ) ? m_hwnd : nullptr;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// UIComponentBase.cpp