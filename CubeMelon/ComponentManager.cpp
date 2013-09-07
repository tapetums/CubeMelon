// ComponentManager.cpp

#include <memory>
#include <map>
#include <vector>

#include <windows.h>
#include <strsafe.h>

#include <shlwapi.h>
#pragma comment(lib, "ole32.lib")
#include <propsys.h>
#pragma comment(lib, "shlwapi.lib")

#include "..\include\ComPtr.h"
#include "..\include\DebugPrint.h"
#include "..\include\Functions.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"

#include "ComponentManager.h"

//---------------------------------------------------------------------------//

#define MSG_01_E TEXT("Cannot find component directory.\nWould you like to create it?")
#define MSG_01_J TEXT("components フォルダが見つかりません。\n作成しますか？")
#define MSG_01 MSG_01_J

#define NAME1 TEXT("ComponentManager")
#define NAME2 TEXT("ComponentContainer")

#define MAX_COMPONENT_COUNT 256

//---------------------------------------------------------------------------//

inline bool operator<(const GUID& lhs, const GUID& rhs)
{
    return memcmp(&lhs, &rhs, sizeof(IID)) < 0;
}

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct ComponentContainer::Impl
{
    Impl(LPCWSTR file_path, size_t index);
    ~Impl();
    void    __stdcall Free();
    HRESULT __stdcall GetTypicalProperties();

    size_t            index;
    LPWSTR            copyright;
    LPWSTR            description;
    LPWSTR            name;
    HMODULE           hModule;
    DLLCANUNLOADNOW   DllCanUnloadNow;
    DLLCONFIGURE      DllConfigure;
    DLLGETPROPERTY    DllGetProperty;
    DLLGETCLASSOBJECT DllGetClassObject;
    CLSID             clsid;
    VersionInfo       ver;
    WCHAR             file_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

ComponentContainer::Impl::Impl(LPCWSTR file_path, size_t index)
{
    DebugPrintLn(NAME2 TEXT("::Impl::Constructor() begin"));

    ::StringCchPrintf(this->file_path, MAX_PATH, file_path);
    this->index = index;

    copyright         = nullptr;
    description       = nullptr;
    name              = nullptr;
    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllGetProperty    = nullptr;
    DllGetClassObject = nullptr;
    clsid             = CLSID_NULL;
    ::ZeroMemory(&ver, sizeof(VersionInfo));

    DebugPrintLn(NAME2 TEXT("::Impl::Constructor() end"));
}

//---------------------------------------------------------------------------//

ComponentContainer::Impl::~Impl()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Destructor() begin"));

    this->Free();

    if ( copyright )
    {
        ::CoTaskMemFree(copyright);
        copyright = nullptr;
    }
    if ( description )
    {
        ::CoTaskMemFree(description);
        description = nullptr;
    }
    if ( name )
    {
        ::CoTaskMemFree(name);
        name = nullptr;
    }

    DebugPrintLn(NAME2 TEXT("::Impl::Destructor() end"));
}

//---------------------------------------------------------------------------//

void __stdcall ComponentContainer::Impl::Free()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Free() begin"));

    if ( nullptr == hModule )
    {
        DebugPrintLn(TEXT("Nothing to do"));
        return;
    }

    DebugPrintLn(TEXT("Freeing DLL..."));
    DebugPrintLn(file_path);

    ::FreeLibrary(hModule);

    DebugPrintLn(TEXT("Freed DLL"));

    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllGetProperty    = nullptr;
    DllGetClassObject = nullptr;

    DebugPrintLn(NAME2 TEXT("::Impl::Free() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::Impl::GetTypicalProperties()
{
    HRESULT     hr;
    PROPERTYKEY key;
    PROPVARIANT pv;
    size_t      wc_count;

    DebugPrintLn(NAME2 TEXT("::Impl::GetTypicalProperties() begin"));

    // IPropertyStore オブジェクトを取得
    IPropertyStore* ps = nullptr;
    hr = DllGetProperty(index, &ps);
    if ( FAILED(hr) || nullptr == ps )
    {
        DebugPrintLn(TEXT("No object at index == %d"), index);
        return hr;
    }

    // クラスIDを取得
    {
        DebugPrintLn(TEXT("Getting ClassID..."));
        key.fmtid = PKEY_CubeMelon_GetProperty;
        key.pid   = CUBEMELON_PID_CLSID;
        ::PropVariantInit(&pv);
        hr = ps->GetValue(key, &pv);
        if ( FAILED(hr) || pv.vt != VT_CLSID )
        {
            DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
            return hr;
        }
        clsid = *pv.puuid;
        ::PropVariantClear(&pv); // なるべくすぐに pv を解放

        // クラスIDを文字列化
        DebugPrintLn
        (
            TEXT("{ %00000000X-%0000X-%0000X-%00X%00X-%00X%00X%00X%00X%00X%00X }"),
            clsid.Data1, clsid.Data2, clsid.Data3,
            clsid.Data4[0], clsid.Data4[1], clsid.Data4[2], clsid.Data4[3],
            clsid.Data4[4], clsid.Data4[5], clsid.Data4[6], clsid.Data4[7]
        );
    }

    // コンポーネント名を取得
    {
        DebugPrintLn(TEXT("Getting component name..."));
        key.fmtid = PKEY_CubeMelon_GetProperty;
        key.pid   = CUBEMELON_PID_NAME;
        ::PropVariantInit(&pv);
        hr = ps->GetValue(key, &pv);
        if ( FAILED(hr) || pv.vt != VT_LPWSTR )
        {
            DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
            return hr;
        }

        wc_count = 1 + wcslen(pv.pwszVal); // NULL文字分を絶対に忘れない！
        name = (LPWSTR)::CoTaskMemAlloc(sizeof(WCHAR) * (wc_count));
        ::StringCchPrintf(name, wc_count, pv.pwszVal);
        ::PropVariantClear(&pv); // なるべくすぐに pv を解放
        DebugPrintLn(name);
    }

    // コンポーネントの詳細説明を取得
    {
        DebugPrintLn(TEXT("Getting component description..."));
        key.fmtid = PKEY_CubeMelon_GetProperty;
        key.pid   = CUBEMELON_PID_DESCRIPTION;
        ::PropVariantInit(&pv);
        hr = ps->GetValue(key, &pv);
        if ( FAILED(hr) || pv.vt != VT_LPWSTR )
        {
            DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
            return hr;
        }

        wc_count = 1 + wcslen(pv.pwszVal); // NULL文字分を絶対に忘れない！
        description = (LPWSTR)::CoTaskMemAlloc(sizeof(WCHAR) * (wc_count));
        ::StringCchPrintf(description, wc_count, pv.pwszVal);
        ::PropVariantClear(&pv); // なるべくすぐに pv を解放
        DebugPrintLn(description);
    }

    // コンポーネントの著作権情報を取得
    {
        DebugPrintLn(TEXT("Getting component copyright information..."));
        key.fmtid = PKEY_CubeMelon_GetProperty;
        key.pid   = CUBEMELON_PID_COPYRIGHT;
        ::PropVariantInit(&pv);
        hr = ps->GetValue(key, &pv);
        if ( FAILED(hr) || pv.vt != VT_LPWSTR )
        {
            DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
            return hr;
        }

        wc_count = 1 + wcslen(pv.pwszVal); // NULL文字分を絶対に忘れない！
        copyright = (LPWSTR)::CoTaskMemAlloc(sizeof(WCHAR) * (wc_count));
        ::StringCchPrintf(copyright, wc_count, pv.pwszVal);
        ::PropVariantClear(&pv); // なるべくすぐに pv を解放
        DebugPrintLn(copyright);
    }

    // コンポーネントのバージョン情報を取得
    {
        DebugPrintLn(TEXT("Getting component version..."));
        key.fmtid = PKEY_CubeMelon_GetProperty;
        key.pid   = CUBEMELON_PID_VERSION;
        ::PropVariantInit(&pv);
        hr = ps->GetValue(key, &pv);
        if ( FAILED(hr) || pv.vt != VT_PTR )
        {
            DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
            return hr;
        }

        ver = *((VersionInfo*)pv.pintVal);
        ::PropVariantClear(&pv); // なるべくすぐに pv を解放
        DebugPrintLn
        (
            TEXT("%d.%d.%d %c"),
            ver.major, ver.minor, ver.revision, ver.stage
        );
    }

    // IPropertyStore オブジェクトを解放
    ps->Release();
    ps = nullptr;

    DebugPrintLn(NAME2 TEXT("::Impl::GetTypicalProperties() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ComponentContainer::ComponentContainer(LPCWSTR file_path, size_t index)
{
    DebugPrintLn(NAME2 TEXT("::Constructor() begin"));

    pimpl = new Impl(file_path, index);

    m_cRef = 0;

    this->AddRef();

    DebugPrintLn(NAME2 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

ComponentContainer::~ComponentContainer()
{
    DebugPrintLn(NAME2 TEXT("::Destructor() begin"));

    this->Free();

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME2 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME2 TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IComponentContainer) )
    {
        *ppvObject = static_cast<IComponentContainer*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(NAME2 TEXT("::QueryInterface() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall ComponentContainer::AddRef()
{
    DebugPrintLn(NAME2 TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME2 TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall ComponentContainer::Release()
{
    DebugPrintLn(NAME2 TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME2 TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall ComponentContainer::ClassID() const
{
    return pimpl->clsid;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall ComponentContainer::Copyright() const
{
    return pimpl->copyright;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall ComponentContainer::Description() const
{
    return pimpl->description;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall ComponentContainer::FilePath() const
{
    return pimpl->file_path;
}

//---------------------------------------------------------------------------//

size_t __stdcall ComponentContainer::Index() const
{
    return pimpl->index;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall ComponentContainer::Name() const
{
    return pimpl->name;
}

//---------------------------------------------------------------------------//

VersionInfo* __stdcall ComponentContainer::Version() const
{
    return &pimpl->ver;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::Load()
{
    DebugPrintLn(NAME2 TEXT("::Load() begin"));
    DebugPrintLn(pimpl->file_path);

    if ( pimpl->hModule )
    {
        DebugPrintLn(TEXT("Already loaded"));
        return S_FALSE;
    }

    try
    {
        // DLLの読み込み
        pimpl->hModule = ::LoadLibraryEx
        (
            pimpl->file_path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
        );
        if ( nullptr == pimpl->hModule )
        {
            throw TEXT("Failed to load component");
        }

        // 公開関数のアドレスを取得
        pimpl->DllCanUnloadNow = (DLLCANUNLOADNOW)::GetProcAddress
        (
            pimpl->hModule, "DllCanUnloadNow"
        );
        if ( nullptr == pimpl->DllCanUnloadNow )
        {
            throw TEXT("DllCanUnloadNow() is not found");
        }

        pimpl->DllConfigure = (DLLCONFIGURE)::GetProcAddress
        (
            pimpl->hModule, "DllConfigure"
        );
        if ( nullptr == pimpl->DllConfigure )
        {
            throw TEXT("DllConfigure() is not found");
        }

        pimpl->DllGetProperty = (DLLGETPROPERTY)::GetProcAddress
        (
            pimpl->hModule, "DllGetProperty"
        );
        if ( nullptr == pimpl->DllGetProperty )
        {
            throw TEXT("DllGetProperty() is not found");
        }

        pimpl->DllGetClassObject = (DLLGETCLASSOBJECT)::GetProcAddress
        (
            pimpl->hModule, "DllGetClassObject"
        );
        if ( nullptr == pimpl->DllGetClassObject )
        {
            throw TEXT("DllGetClassObject() is not found");
        }
    }
    catch (LPCWSTR msg)
    {
        DebugPrintLn(msg);
        msg = nullptr;

        pimpl->Free();

        return E_FAIL;
    }

    // ClassIDなど、主なコンポーネント情報を取得
    auto hr = pimpl->GetTypicalProperties();
    if ( FAILED(hr) )
    {
        return E_FAIL;
    }

    DebugPrintLn(NAME2 TEXT("::Load() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::Free()
{
    DebugPrintLn(NAME2 TEXT("::Free() begin"));
    DebugPrintLn(pimpl->file_path);

    if ( nullptr == pimpl->hModule )
    {
        DebugPrintLn(TEXT("Already Freed"));
        return S_FALSE;
    }

    auto hr = pimpl->DllCanUnloadNow();
    if ( S_OK != hr )
    {
        DebugPrintLn(TEXT("Module is locked"));
        return E_FAIL;
    }

    pimpl->Free();

    DebugPrintLn(NAME2 TEXT("::Free() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::Configure(HWND hwndParent)
{
    return pimpl->DllConfigure(hwndParent);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::CreateInstance
(
    IUnknown* pUnkOuter, REFIID riid, void** ppvObject
)
{
    HRESULT hr;

    DebugPrintLn(NAME2 TEXT("::CreateInstance() begin"));

    if ( nullptr == pimpl->hModule )
    {
        hr = this->Load();
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    IClassFactory* factory = nullptr;
    hr = pimpl->DllGetClassObject
    (
        pimpl->clsid, IID_IClassFactory, (void**)&factory
    );
    if ( FAILED(hr) || nullptr == factory )
    {
        return hr;
    }

    hr = factory->CreateInstance
    (
        pUnkOuter, riid, ppvObject
    );
    if ( FAILED(hr) || nullptr == *ppvObject )
    {
        return hr;
    }
    factory->Release();
    factory = nullptr;

    DebugPrintLn(NAME2 TEXT("::CreateInstance() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::GetProperty(IPropertyStore** ps)
{
    return pimpl->DllGetProperty(pimpl->index, ps);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentContainer::SetProperty(const IPropertyStore* ps)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

typedef std::map<CLSID, ComPtr<ComponentContainer>> ComponentMap;

struct ComponentManager::Impl
{
    Impl();
    ~Impl();
    void __stdcall Reset();

    void __stdcall ScanDirectory(LPCWSTR dir_path);
    void __stdcall RegisterComponent(LPCWSTR comp_path);

    size_t               count;
    ComponentContainer** comps;
    ComponentMap         cmap;
    WCHAR                dir_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

ComponentManager::Impl::Impl()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Constructor() begin"));

    count       = 0;
    comps       = nullptr;
    dir_path[0] = '\0';

    DebugPrintLn(NAME1 TEXT("::Impl::Constructor() end"));
}

//---------------------------------------------------------------------------//

ComponentManager::Impl::~Impl()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Destructor() begin"));

    this->Reset();

    DebugPrintLn(NAME1 TEXT("::Impl::Destructor() end"));
}

//---------------------------------------------------------------------------//

void __stdcall ComponentManager::Impl::Reset()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Reset() begin"));

    dir_path[0] = '\0';

    cmap.clear();

    if ( comps )
    {
        for ( size_t index = 0; index < count; ++index )
        {
            comps[index]->Release();
            comps[index] = nullptr;
        }
        delete[] comps;
        comps = nullptr;
    }

    count = 0;

    DebugPrintLn(NAME1 TEXT("::Impl::Reset() end"));
}

//---------------------------------------------------------------------------//

void __stdcall ComponentManager::Impl::ScanDirectory(LPCWSTR dir_path)
{
    DebugPrintLn(NAME1 TEXT("::ScanDirectory() begin"));

    WCHAR path[MAX_PATH];
    ::StringCchPrintf(path, MAX_PATH, TEXT("%s\\*"), dir_path);
    DebugPrintLn(TEXT("Scanning..."));
    DebugPrintLn(path);

    // フォルダ内にあるものを列挙する
    WIN32_FIND_DATA fd = { 0 };
    auto hFindFile = ::FindFirstFile(path, &fd);
    if ( hFindFile == INVALID_HANDLE_VALUE )
    {
        auto ret = ::MessageBox
        (
            nullptr, MSG_01, nullptr, MB_YESNO | MB_DEFBUTTON2
        );
        if ( ret == IDYES )
        {
            DebugPrintLn(TEXT("Creating components directory..."));
            ::CreateDirectory(dir_path, nullptr);
            DebugPrintLn(TEXT("Created."));
        }
        return;
    }

    do
    {
        DebugPrintLn(TEXT("Checking file attribute..."));
        DebugPrintLn(fd.cFileName);

        // ファイル名がピリオドで始まっているものは飛ばす
        if ( fd.cFileName && fd.cFileName[0] == '.' )
        {
            DebugPrintLn(TEXT("This is a dot file"));
            continue;
        }

        // 隠し属性を持つものは飛ばす
        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
        {
            DebugPrintLn(TEXT("This is a hidden file"));
            continue;
        }

        DebugPrintLn(TEXT("Checked file attribute"));

        // フルパスを合成
        ::StringCchPrintf
        (
            path, MAX_PATH, TEXT("%s\\%s"), dir_path, fd.cFileName
        );

        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            // フォルダだったら
            if ( true )
            {
                // サブフォルダを検索
                ScanDirectory(path);
            }
        }
        else
        {
            // DLLファイルだったら
            if ( lstrcmp(::PathFindExtension(path), TEXT(".dll")) == 0 )
            {
                // コンポーネントデータベースに記憶
                RegisterComponent(path);
            }
        }

        DebugPrintLn(TEXT("Next file"));
    }
    while ( ::FindNextFile(hFindFile, &fd) == TRUE );

    ::FindClose( hFindFile );

    DebugPrintLn(NAME1 TEXT("::ScanDirectory() end"));
}

//---------------------------------------------------------------------------//

void __stdcall ComponentManager::Impl::RegisterComponent(LPCWSTR comp_path)
{
    DebugPrintLn(NAME1 TEXT("::RegisterComponent() begin"));
    DebugPrintLn(comp_path);

    ComponentContainer* cc = nullptr;
    CLSID clsid = CLSID_NULL;

    // DLLファイルに含まれているクラスIDを全て取得
    for ( size_t index = 0; index < MAX_COMPONENT_COUNT ; ++index )
    {
        DebugPrintLn(TEXT("Searching ClassIDs: index == %d"), index);

        // コンポーネント情報を管理するオブジェクトを生成
        cc = new ComponentContainer(comp_path, index);
        auto hr = cc->Load();
        if ( FAILED(hr) )
        {
            DebugPrintLn(TEXT("No more component in this DLL"));
            cc->Release();
            cc = nullptr;
            break;
        }

        // コンポーネントのクラスIDを取得
        clsid = cc->ClassID();

        // 同じクラスIDがすでに登録されていないか確認
        DebugPrintLn(TEXT("Checking ClassID..."));
        {
            auto it = cmap.find(clsid);
            if ( it != cmap.end() )
            {
                DebugPrintLn(TEXT("Duplicate ClassID"));
                cc->Release();
                cc = nullptr;
                continue;
            }
        }
        DebugPrintLn(TEXT("Checked ClassID: no duplicate"));

        // コンポーネント情報を本体のデータベースに記憶
        DebugPrintLn(TEXT("Registering component..."));
        {
            cmap.emplace
            (
                std::make_pair
                (
                    clsid, ComPtr<ComponentContainer>(cc)
                )
            );
        }
        DebugPrintLn(TEXT("Registered component"));
    }
    DebugPrintLn(TEXT("Searched ClassIDs"));

    DebugPrintLn(NAME1 TEXT("::RegisterComponent() end"));
}

//---------------------------------------------------------------------------//

ComponentManager::ComponentManager(LPCWSTR dir_path)
{
    DebugPrintLn(NAME1 TEXT("::Constructor() begin"));

    pimpl = new Impl;
    ::StringCchPrintf(pimpl->dir_path, MAX_PATH, dir_path);

    m_cRef = 0;

    this->AddRef();

    DebugPrintLn(NAME1 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

ComponentManager::~ComponentManager()
{
    DebugPrintLn(NAME1 TEXT("::Destructor() begin"));

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME1 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentManager::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME1 TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IComponentManager) )
    {
        *ppvObject = static_cast<IComponentManager*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(NAME1 TEXT("::QueryInterface() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall ComponentManager::AddRef()
{
    DebugPrintLn(NAME1 TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME1 TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall ComponentManager::Release()
{
    DebugPrintLn(NAME1 TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME1 TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

IComponentContainer* __stdcall ComponentManager::At(size_t index) const
{
    if ( index < pimpl->count )
    {
        return static_cast<IComponentContainer*>(pimpl->comps[index]);
    }
    else
    {
        return nullptr;
    }
}

//---------------------------------------------------------------------------//

size_t __stdcall ComponentManager::ComponentCount() const
{
    return pimpl->count;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall ComponentManager::DirectoryPath() const
{
    return pimpl->dir_path;
}

//---------------------------------------------------------------------------//

IComponentContainer* __stdcall ComponentManager::Find(REFCLSID rclsid) const
{
    DebugPrintLn(NAME1 TEXT("::Find()begin"));

    IComponentContainer* cc = nullptr;
    auto it = pimpl->cmap.find(rclsid);
    if ( it != pimpl->cmap.end() )
    {
        cc = it->second.GetInterface();
        cc->AddRef();
    }

    DebugPrintLn(NAME1 TEXT("::Find() end"));

    return cc;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentManager::LoadAll()
{
    DebugPrintLn(NAME1 TEXT("::LoadAll() begin"));

    // コンポーネントデータベースを一旦破棄
    if ( pimpl->count )
    {
        DebugPrintLn(TEXT("Resetting component database..."));

        auto hr = this->FreeAll();
        if ( hr != S_OK )
        {
            DebugPrintLn(TEXT("Couldn't start loading"));
            return E_FAIL;
        }

        DebugPrintLn(TEXT("Resetted component database"));
    }

    // コンポーネントフォルダをスキャン
    pimpl->ScanDirectory(pimpl->dir_path);
    pimpl->count = pimpl->cmap.size();
    if ( pimpl->count < 1 )
    {
        DebugPrintLn(TEXT("No component found"));
    }

    // コンポーネントデータベースを構築
    pimpl->comps = new ComponentContainer*[1 + pimpl->count]; // 0でも空の配列を作る

    auto it  = pimpl->cmap.begin();
    auto end = pimpl->cmap.end();
    for ( size_t i = 0; it != end; ++i, ++it )
    {
        pimpl->comps[i] = it->second.GetInterface();
    }
    pimpl->comps[pimpl->count] = nullptr; // 配列はNULL終端

    DebugPrintLn(NAME1 TEXT("::LoadAll() end"));

    return pimpl->count ? S_OK : S_FALSE;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall ComponentManager::FreeAll()
{
    DebugPrintLn(NAME1 TEXT("::FreeAll() begin"));

    HRESULT hr;
    size_t freed = 0;

    auto count = pimpl->count;
    for ( size_t i = 0; i < count; ++i )
    {
        hr = pimpl->comps[i]->Free();
        if ( SUCCEEDED(hr) )
        {
            ++freed;
        }
    }
    if ( freed == 0 )
    {
        DebugPrintLn(TEXT("Freed nothing"));
        hr = E_FAIL;
    }
    else if ( freed == count )
    {
        DebugPrintLn(TEXT("Freed all"));
        hr = S_OK;
    }
    else
    {
        DebugPrintLn(TEXT("Not freed all"));
        hr = S_FALSE;
    }

    DebugPrintLn(NAME1 TEXT("::FreeAll() end"));

    return hr;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// ComponentManager.cpp