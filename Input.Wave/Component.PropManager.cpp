// Input.Wave.Component.PropManager.cpp

#include <windows.h>
#include <strsafe.h>
#include <mmreg.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"

#include "Component.PropManager.h"

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component;

extern const size_t   COMP_PROP_COUNT    = 2;
extern const wchar_t* COMP_PROP_MGR_NAME = TEXT("Input.Wave.Component.PropManager");
extern const wchar_t* COMP_PROP_NAME     = TEXT("Input.Wave.Component.Property");

extern const wchar_t* COMP_PROP_PATH     = TEXT("Path");
extern const wchar_t* COMP_PROP_FORMAT   = TEXT("Wave Format");

//---------------------------------------------------------------------------//

// 関数ポインタ型の宣言
typedef void (*MakePropFunc)(IProperty**, LPCVOID);

//---------------------------------------------------------------------------//

void MakePath(IProperty** prop, LPCVOID var)
{
    DebugPrintLn(TEXT("MakePath begin"));

    auto path = (WCHAR*)var;
    auto cb   = sizeof(WCHAR) * (1 + wcslen(path));
    auto data = new VARIANT;
    auto buf = (WCHAR*)new BYTE[cb];
    ::StringCbCopy(buf, cb, path);
    data->wstring = buf;

    *prop = new CompProperty
    (
        COMP_PROP_PATH, TYPE_WSTRING, data, cb
    );

    DebugPrintLn(TEXT("MakePath end"));
}

//---------------------------------------------------------------------------//

void MakeWaveFormat(IProperty** prop, LPCVOID var)
{
    DebugPrintLn(TEXT("MakeWaveFormat begin"));

    size_t cb = 0;
    auto data = new VARIANT;
    data->ptr = new WAVEFORMATEXTENSIBLE;

    ::ZeroMemory(data->ptr, sizeof(WAVEFORMATEXTENSIBLE));
    if ( var )
    {
        auto tag = ((WAVEFORMATEXTENSIBLE*)var)->Format.wFormatTag;
        if ( tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_IEEE_FLOAT )
        {
            cb = sizeof(PCMWAVEFORMAT);
        }
        else if ( tag == WAVE_FORMAT_EXTENSIBLE )
        {
            cb = sizeof(WAVEFORMATEXTENSIBLE);
        }
    }
    ::CopyMemory(data->ptr, var, cb);

    *prop = new CompProperty
    (
        COMP_PROP_FORMAT, TYPE_PTR, data, cb
    );

    DebugPrintLn(TEXT("MakeWaveFormat end"));
}

//---------------------------------------------------------------------------//

CompProperty::CompProperty(LPCWSTR name, DATATYPE type, VARIANT* data, size_t size)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_PROP_NAME);

    m_cRef = 0;

    m_data = data;
    m_size = size;
    m_type = type;

    auto cch = 1 + wcslen(name);
    m_name = new WCHAR[cch];
    ::StringCchCopy(m_name, cch, name);

    this->AddRef();

    DebugPrintLn(TEXT("%s::Constructor() end"), COMP_PROP_NAME);
}

//---------------------------------------------------------------------------//

CompProperty::~CompProperty()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), COMP_PROP_NAME);

    if ( m_data )
    {
        delete m_data;
        m_data = nullptr;
    }
    if ( m_name )
    {
        delete[] m_name;
        m_name = nullptr;
    }

    DebugPrintLn(TEXT("%s::Destructor() end"), COMP_PROP_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompProperty::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(TEXT("%s::QueryInterface() begin"), COMP_PROP_NAME);

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IProperty) )
    {
        *ppvObject = static_cast<IProperty*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(TEXT("%s::QueryInterface() end"), COMP_PROP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall CompProperty::AddRef()
{
    DebugPrintLn(TEXT("%s::AddRef() begin %d"), COMP_PROP_NAME, m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(TEXT("%s::AddRef() end %d"), COMP_PROP_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall CompProperty::Release()
{
    DebugPrintLn(TEXT("%s::Release() begin %d"), COMP_PROP_NAME, m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(TEXT("%s::Release() end %d"), COMP_PROP_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

VARIANT* __stdcall CompProperty::Data() const
{
    return m_data;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall CompProperty::Name() const
{
    return m_name;
}

//---------------------------------------------------------------------------//

size_t __stdcall CompProperty::Size() const
{
    return m_size;
}

//---------------------------------------------------------------------------//

DATATYPE __stdcall CompProperty::Type() const
{
    return m_type;
}

//---------------------------------------------------------------------------//

CompPropManager::CompPropManager(LPCWSTR path, WAVEFORMATEXTENSIBLE* wfex)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_PROP_MGR_NAME);

    m_cRef = 0;

    m_path = path;
    m_wfex = wfex;

    this->AddRef();

    DebugPrintLn(TEXT("%s::Constructor() end"), COMP_PROP_MGR_NAME);
}

//---------------------------------------------------------------------------//

CompPropManager::~CompPropManager()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), COMP_PROP_MGR_NAME);

    m_wfex = nullptr;

    DebugPrintLn(TEXT("%s::Destructor() end"), COMP_PROP_MGR_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompPropManager::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(TEXT("%s::QueryInterface() begin"), COMP_PROP_MGR_NAME);

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPropManager) )
    {
        *ppvObject = static_cast<IPropManager*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(TEXT("%s::QueryInterface() end"), COMP_PROP_MGR_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall CompPropManager::AddRef()
{
    DebugPrintLn(TEXT("%s::AddRef() begin %d"), COMP_PROP_MGR_NAME, m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(TEXT("%s::AddRef() end %d"), COMP_PROP_MGR_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall CompPropManager::Release()
{
    DebugPrintLn(TEXT("%s::Release() begin %d"), COMP_PROP_MGR_NAME, m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(TEXT("%s::Release() end %d"), COMP_PROP_MGR_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

size_t __stdcall CompPropManager::PropCount() const
{
   return COMP_PROP_COUNT;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompPropManager::GetAt
(
    size_t index, IProperty** prop
)
{
    DebugPrintLn(TEXT("%s::GetAt() begin"), COMP_PROP_MGR_NAME);

    if ( nullptr == prop )
    {
        return E_POINTER;
    }

    if ( index == 0 )
    {
        MakePath(prop, m_path);
    }
    else if ( index == 1 )
    {
        MakeWaveFormat(prop, m_wfex);
    }
    else//if ( index >= COMP_PROP_COUNT )
    {
        return E_INVALIDARG;
    }

    DebugPrintLn(TEXT("%s::GetAt() end"), COMP_PROP_MGR_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompPropManager::GetByName
(
    LPCWSTR name, IProperty** prop
)
{
    DebugPrintLn(TEXT("%s::GetByName() begin"), COMP_PROP_MGR_NAME);

    if ( nullptr == prop )
    {
        return E_POINTER;
    }

    HRESULT hr = E_FAIL;

    if ( lstrcmp(name, COMP_PROP_PATH) == 0 )
    {
        MakePath(prop, m_path);
        hr = S_OK;
    }
    else if ( lstrcmp(name, COMP_PROP_FORMAT) == 1 )
    {
        MakeWaveFormat(prop, m_wfex);
        hr = S_OK;
    }

    DebugPrintLn(TEXT("%s::GetByName() end"), COMP_PROP_MGR_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompPropManager::SetAt
(
    size_t index, IProperty* prop
)
{
    DebugPrintLn(TEXT("%s::SetAt() begin"), COMP_PROP_MGR_NAME);

    DebugPrintLn(TEXT("%s::SetAt() end"), COMP_PROP_MGR_NAME);

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompPropManager::SetByName
(
    LPCWSTR name, IProperty* prop
)
{
    DebugPrintLn(TEXT("%s::SetByName() begin"), COMP_PROP_MGR_NAME);

    DebugPrintLn(TEXT("%s::SetByName() end"), COMP_PROP_MGR_NAME);


    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.Component.PropManager.cpp