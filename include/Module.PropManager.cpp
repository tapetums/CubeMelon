// Module.PropManager.cpp

#include <map>
#include <string>

#include <windows.h>
#include <strsafe.h>

#include "DebugPrint.h"
#include "LockModule.h"
#include "Interfaces.h"

#include "PropManager.h"

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component;

extern const size_t      MDL_PROP_COUNT;
extern const wchar_t*    MDL_PROP_MGR_NAME;
extern const wchar_t*    MDL_PROP_NAME;

extern const wchar_t*    PropName;
extern const wchar_t*    PropDescription;
extern const wchar_t*    PropCopyright;
extern const VersionInfo PropVersion;

//---------------------------------------------------------------------------//

// 関数ポインタ型の宣言
typedef void (*MakePropFunc)(IProperty**);

//---------------------------------------------------------------------------//

// MSがC++11の機能をなかなか実装してくれないので仕方なく
struct GlobalVarinat
{
    GlobalVarinat();
    ~GlobalVarinat();

    std::map<std::wstring, MakePropFunc> func_map;
    MakePropFunc* func_array;
};

// std::map の初期化を静的に行うためグローバル変数化
static const GlobalVarinat global;

// ホントは構造体を使わずこう書きたい：
/*
func_array[] =
{
    MakeClassID, MakeName, MakeDescription, MakeCopyrightInfo, MakeVersionInfo
};

std::map<std::wstring, MakePropFunc> func_map =
{
    { TEXT("ClassID"),     MakeClassID       },
    { TEXT("Name"),        MakeName          },
    { TEXT("Description"), MakeDescription   },
    { TEXT("Copyright"),   MakeCopyrightInfo },
    { TEXT("VersionInfo"), MakeVersionInfo   }
};*/

//---------------------------------------------------------------------------//

void MakeClassID(IProperty** prop)
{
    DebugPrintLn(TEXT("MakeClassID begin"));

    auto data = new VARIANT;
    data->clsid = CLSID_Component;

    *prop = new Property
    (
        TEXT("ClassID"), TYPE_CLSID, data, sizeof(CLSID)
    );

    DebugPrintLn(TEXT("MakeClassID end"));
}

//---------------------------------------------------------------------------//

void MakeName(IProperty** prop)
{
    DebugPrintLn(TEXT("MakeName begin"));

    auto cb = sizeof(WCHAR) * (1 + wcslen(PropName));
    auto data = new VARIANT;
    data->wstring = PropName;

    *prop = new Property
    (
        TEXT("Name"), TYPE_WSTRING, data, cb
    );

    DebugPrintLn(TEXT("MakeName end"));
}

//---------------------------------------------------------------------------//

void MakeDescription(IProperty** prop)
{
    DebugPrintLn(TEXT("MakeDescription begin"));

    auto cb = sizeof(WCHAR) * (1 + wcslen(PropDescription));
    auto data = new VARIANT;
    data->wstring = PropDescription;

    *prop = new Property
    (
        TEXT("Description"), TYPE_WSTRING, data, cb
    );

    DebugPrintLn(TEXT("MakeDescription end"));
}

//---------------------------------------------------------------------------//

void MakeCopyrightInfo(IProperty** prop)
{
    DebugPrintLn(TEXT("MakeCopyrightInfo begin"));

    auto cb = sizeof(WCHAR) * (1 + wcslen(PropCopyright));
    auto data = new VARIANT;
    data->wstring = PropCopyright;

    *prop = new Property
    (
        TEXT("Copyright"), TYPE_WSTRING, data, cb
    );

    DebugPrintLn(TEXT("MakeCopyrightInfo end"));
}

//---------------------------------------------------------------------------//

void MakeVersionInfo(IProperty** prop)
{
    DebugPrintLn(TEXT("MakeVersionInfo begin"));

    auto cb = sizeof(VersionInfo);
    auto data = new VARIANT;
    data->version = (VersionInfo*)&PropVersion;

    *prop = new Property
    (
        TEXT("VersionInfo"), TYPE_VERSIONINFO, data, cb
    );

    DebugPrintLn(TEXT("MakeVersionInfo end"));
}

//---------------------------------------------------------------------------//

GlobalVarinat::GlobalVarinat()
{
    func_map[TEXT("ClassID")]     = MakeClassID;
    func_map[TEXT("Name")]        = MakeName;
    func_map[TEXT("Description")] = MakeDescription;
    func_map[TEXT("Copyright")]   = MakeCopyrightInfo;
    func_map[TEXT("VersionInfo")] = MakeVersionInfo;

    func_array = new MakePropFunc[MDL_PROP_COUNT];
    func_array[0] = MakeClassID;
    func_array[1] = MakeName;
    func_array[2] = MakeDescription;
    func_array[3] = MakeCopyrightInfo;
    func_array[4] = MakeVersionInfo;
}

//---------------------------------------------------------------------------//

GlobalVarinat::~GlobalVarinat()
{
    func_map.clear();

    delete[] func_array;
}

//---------------------------------------------------------------------------//

Property::Property(LPCWSTR name, DATATYPE type, VARIANT* data, size_t size)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), MDL_PROP_NAME);

    m_cRef = 0;

    m_data = data;
    m_size = size;
    m_type = type;

    auto cch = 1 + wcslen(name);
    m_name = new WCHAR[cch];
    ::StringCchCopy(m_name, cch, name);

    this->AddRef();

    DebugPrintLn(TEXT("%s::Constructor() end"), MDL_PROP_NAME);
}

//---------------------------------------------------------------------------//

Property::~Property()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), MDL_PROP_NAME);

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

    DebugPrintLn(TEXT("%s::Destructor() end"), MDL_PROP_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Property::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(TEXT("%s::QueryInterface() begin"), MDL_PROP_NAME);

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

    DebugPrintLn(TEXT("%s::QueryInterface() end"), MDL_PROP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall Property::AddRef()
{
    DebugPrintLn(TEXT("%s::AddRef() begin %d"), MDL_PROP_NAME, m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(TEXT("%s::AddRef() end %d"), MDL_PROP_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Property::Release()
{
    DebugPrintLn(TEXT("%s::Release() begin %d"), MDL_PROP_NAME, m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(TEXT("%s::Release() end %d"), MDL_PROP_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

VARIANT* __stdcall Property::Data() const
{
    return m_data;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall Property::Name() const
{
    return m_name;
}

//---------------------------------------------------------------------------//

size_t __stdcall Property::Size() const
{
    return m_size;
}

//---------------------------------------------------------------------------//

DATATYPE __stdcall Property::Type() const
{
    return m_type;
}

//---------------------------------------------------------------------------//

PropManager::PropManager()
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), MDL_PROP_MGR_NAME);

    m_cRef = 0;

    this->AddRef();

    DebugPrintLn(TEXT("%s::Constructor() end"), MDL_PROP_MGR_NAME);
}

//---------------------------------------------------------------------------//

PropManager::~PropManager()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), MDL_PROP_MGR_NAME);

    DebugPrintLn(TEXT("%s::Destructor() end"), MDL_PROP_MGR_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropManager::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(TEXT("%s::QueryInterface() begin"), MDL_PROP_MGR_NAME);

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

    DebugPrintLn(TEXT("%s::QueryInterface() end"), MDL_PROP_MGR_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall PropManager::AddRef()
{
    DebugPrintLn(TEXT("%s::AddRef() begin %d"), MDL_PROP_MGR_NAME, m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(TEXT("%s::AddRef() end %d"), MDL_PROP_MGR_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall PropManager::Release()
{
    DebugPrintLn(TEXT("%s::Release() begin %d"), MDL_PROP_MGR_NAME, m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(TEXT("%s::Release() end %d"), MDL_PROP_MGR_NAME, cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

size_t __stdcall PropManager::PropCount() const
{
   return MDL_PROP_COUNT;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropManager::GetAt
(
    size_t index, IProperty** prop
)
{
    DebugPrintLn(TEXT("%s::GetAt() begin"), MDL_PROP_MGR_NAME);

    if ( nullptr == prop )
    {
        return E_POINTER;
    }

    if ( index >= MDL_PROP_COUNT )
    {
        return E_INVALIDARG;
    }

    global.func_array[index](prop);

    DebugPrintLn(TEXT("%s::GetAt() end"), MDL_PROP_MGR_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropManager::GetByName
(
    LPCWSTR name, IProperty** prop
)
{
    DebugPrintLn(TEXT("%s::GetByName() begin"), MDL_PROP_MGR_NAME);

    if ( nullptr == prop )
    {
        return E_POINTER;
    }

    HRESULT hr = E_FAIL;

    auto it = global.func_map.find(std::wstring(name));
    if ( it != global.func_map.end() )
    {
        auto func = it->second;
        if ( func )
        {
            func(prop);
            hr = S_OK;
        }
    }

    DebugPrintLn(TEXT("%s::GetByName() end"), MDL_PROP_MGR_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropManager::SetAt
(
    size_t index, IProperty* prop
)
{
    DebugPrintLn(TEXT("%s::SetAt() begin"), MDL_PROP_MGR_NAME);

    DebugPrintLn(TEXT("%s::SetAt() end"), MDL_PROP_MGR_NAME);

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PropManager::SetByName
(
    LPCWSTR name, IProperty* prop
)
{
    DebugPrintLn(TEXT("%s::SetByName() begin"), MDL_PROP_MGR_NAME);

    DebugPrintLn(TEXT("%s::SetByName() end"), MDL_PROP_MGR_NAME);


    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Module.PropManager.cpp