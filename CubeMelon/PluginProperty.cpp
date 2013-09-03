// CubeMelon.PluginProperty.cpp

#include <windows.h>
#include <propsys.h>

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
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginProperty::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginProperty::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    return E_NOTIMPL;
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

// CubeMelon.PluginProperty.cpp