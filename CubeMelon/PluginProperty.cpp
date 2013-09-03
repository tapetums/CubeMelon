﻿// CubeMelon.PluginProperty.cpp

#include <windows.h>
#include <propsys.h>
#include <strsafe.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\PluginProperty.h"

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Plugin =
{ 0x1511e8d0, 0x55a8, 0x4cc2, { 0x94, 0x8b, 0x9f, 0xee, 0xe, 0x63, 0x92, 0x5a } };

//---------------------------------------------------------------------------//

PluginProperty::PluginProperty()
{
}

//---------------------------------------------------------------------------//

PluginProperty::~PluginProperty()
{
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginProperty::QueryInterface(REFIID riid, void** ppvObject)
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

ULONG __stdcall PluginProperty::AddRef()
{
    LockModule();

    return 2;
}

//---------------------------------------------------------------------------//

ULONG __stdcall PluginProperty::Release()
{
    UnlockModule();

    return 1;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginProperty::GetCount(DWORD* cProps)
{
    if ( nullptr == cProps )
   {
       return E_INVALIDARG;
   }

   *cProps = 5;

  return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginProperty::GetAt(DWORD iProp, PROPERTYKEY* pkey)
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

HRESULT __stdcall PluginProperty::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
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
            *puuid = CLSID_Plugin;
            pv->puuid = puuid;
            return S_OK;
        }
        case CUBEMELON_PID_NAME:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("CubeMelon"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_DESCRIPTION:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("Plugin-based Application System"));
            pv->pwszVal = str;
            return S_OK;
        }
        case CUBEMELON_PID_COPYRIGHT:
        {
            pv->vt = VT_LPWSTR;
            auto str = (LPWSTR)::CoTaskMemAlloc(MAX_PATH * sizeof(WCHAR));
            ::StringCchPrintf(str, MAX_PATH, TEXT("(C) 2010-2013 tapetums"));
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

HRESULT __stdcall PluginProperty::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginProperty::Commit()
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

// CubeMelon.PluginProperty.cpp