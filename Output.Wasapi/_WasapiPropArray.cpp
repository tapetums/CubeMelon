// Input.Wave.WaveProperty.cpp

#include <windows.h>
#include <strsafe.h>

#include <propsys.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"

#include "WaveProperty.h"

//---------------------------------------------------------------------------//

#define PID_COUNT 1
#define NAME TEXT("Input.Wave::WaveProperty")

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct WaveProperty::Impl
{
    WAVEFORMATEXTENSIBLE* wfex;
};

//---------------------------------------------------------------------------//

WaveProperty::WaveProperty(WAVEFORMATEXTENSIBLE* wfex)
{
    pimpl = new Impl;
    pimpl->wfex = wfex;

    this->AddRef();
}

//---------------------------------------------------------------------------//

WaveProperty::~WaveProperty()
{
    delete pimpl;
    pimpl = nullptr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WaveProperty::QueryInterface(REFIID riid, void** ppvObject)
{
    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPropertyStore) )
    {
        *ppvObject = static_cast<IPropertyStore*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall WaveProperty::AddRef()
{
    DebugPrintLn(NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall WaveProperty::Release()
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

HRESULT __stdcall WaveProperty::GetCount(DWORD* cProps)
{
     if ( nullptr == cProps )
    {
        return E_POINTER;
    }

    *cProps = PID_COUNT;

   return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WaveProperty::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    if ( nullptr == pkey )
    {
        return E_POINTER;
    }

    if ( iProp >= PID_COUNT )
    {
        return E_INVALIDARG;
    }

    (*pkey).fmtid = PKEY_CubeMelon_GetProperty;
    (*pkey).pid   = iProp + CUBEMELON_PID_NULL;

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WaveProperty::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if ( nullptr == pv )
    {
        return E_POINTER;
    }

    if ( !IsEqualGUID(key.fmtid, PKEY_CubeMelon_GetProperty) )
    {
        return E_FAIL;
    }

    switch ( key.pid )
    {
        case CUBEMELON_PID_NULL:
        {
            pv->vt = VT_PTR;
            auto wfex = (WAVEFORMATEXTENSIBLE*)::CoTaskMemAlloc(sizeof(WAVEFORMATEXTENSIBLE));
            ::CopyMemory(wfex, pimpl->wfex, sizeof(WAVEFORMATEXTENSIBLE));
            pv->puintVal = (UINT_PTR*)wfex;
            return S_OK;
        }
        default:
        {
            return E_FAIL;
        }
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WaveProperty::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall WaveProperty::Commit()
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.WaveProperty.cpp