// UI.SimplePlayer.PropertyStore.cpp

#include <windows.h>
#include <propsys.h>
#include <strsafe.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\PropertyStore.h"

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component =
{ 0x520e13ad, 0x3345, 0x4377, { 0xb2, 0xef, 0x68, 0x42, 0xea, 0x79, 0xb2, 0x5b } };

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

PropertyStore::PropertyStore()
{
}

//---------------------------------------------------------------------------//

PropertyStore::~PropertyStore()
{
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropertyStore::QueryInterface(REFIID riid, void** ppvObject)
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

ULONG __stdcall PropertyStore::AddRef()
{
    LockModule();

    return 2;
}

//---------------------------------------------------------------------------//

ULONG __stdcall PropertyStore::Release()
{
    UnlockModule();

    return 1;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropertyStore::GetCount(DWORD* cProps)
{
     if ( nullptr == cProps )
    {
        return E_INVALIDARG;
    }

    *cProps = 5;

   return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    if ( nullptr == pkey )
    {
        return E_INVALIDARG;
    }

    iProp += CUBEMELON_PID_CLSID;
    if ( iProp < CUBEMELON_PID_CLSID || iProp > CUBEMELON_PID_VERSION )
    {
        return E_FAIL;
    }

    (*pkey).fmtid = PKEY_CubeMelon_GetProperty;
    (*pkey).pid   = iProp;

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if ( nullptr == pv )
    {
        return E_INVALIDARG;
    }

    if ( !IsEqualGUID(key.fmtid, PKEY_CubeMelon_GetProperty) )
    {
        return E_FAIL;
    }

    switch ( key.pid )
    {
        case CUBEMELON_PID_CLSID:
        {
            pv->vt = VT_CLSID;
            auto puuid = (CLSID*)::CoTaskMemAlloc(sizeof(CLSID));
            *puuid = CLSID_Component;
            pv->puuid = puuid;
            return S_OK;
        }
        case CUBEMELON_PID_NAME:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("UI.SimplePlayer"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_DESCRIPTION:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("Audio player window component for CubeMelon"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_COPYRIGHT:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("(C) 2012-2013 tapetums"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_VERSION:
        {
            pv->vt = VT_PTR;
            auto ver = (VersionInfo*)::CoTaskMemAlloc(sizeof(VersionInfo));
            {
                ver->major    = 1;
                ver->minor    = 0;
                ver->revision = 0;
                ver->stage    = 0x61;
            }
            pv->puintVal = (UINT*)ver;
            return S_OK;
        }
        default:
        {
            return E_FAIL;
        }
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropertyStore::Commit()
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// UI.SimplePlayer.PropertyStore.cpp