// Output.Wasapi.ComponentProperty.cpp

#include <windows.h>
#include <propsys.h>
#include <strsafe.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\ComponentProperty.h"

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component =
{ 0xff3fc2e6, 0x2ea5, 0x4c75, { 0x95, 0x92, 0x95, 0x78, 0x7a, 0x22, 0xa8, 0xc5 } };

//---------------------------------------------------------------------------//

ComponentProperty::ComponentProperty()
{
}

//---------------------------------------------------------------------------//

ComponentProperty::~ComponentProperty()
{
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentProperty::QueryInterface(REFIID riid, void** ppvObject)
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

ULONG __stdcall ComponentProperty::AddRef()
{
    LockModule();

    return 2;
}

//---------------------------------------------------------------------------//

ULONG __stdcall ComponentProperty::Release()
{
    UnlockModule();

    return 1;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentProperty::GetCount(DWORD* cProps)
{
     if ( nullptr == cProps )
    {
        return E_INVALIDARG;
    }

    *cProps = 5;

   return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentProperty::GetAt(DWORD iProp, PROPERTYKEY* pkey)
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

HRESULT __stdcall ComponentProperty::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
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
            ::StringCchPrintf(str, MAX_PATH, TEXT("Output.Wasapi"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_DESCRIPTION:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("Output component for Windows Audio Session API"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_COPYRIGHT:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("(C) 2011-2013 tapetums"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_VERSION:
        {
            pv->vt = VT_PTR;
            auto ver = (VersionInfo*)::CoTaskMemAlloc(sizeof(VersionInfo));
            {
                ver->major = 1;
                ver->minor = 0;
                ver->revision = 0;
                ::StringCchPrintf(ver->stage, MAX_PATH, TEXT("alpha"));
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

HRESULT __stdcall ComponentProperty::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentProperty::Commit()
{
    return E_NOTIMPL;
}

// Output.Wasapi.ComponentProperty.cpp