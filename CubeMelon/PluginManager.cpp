// PluginManager.cpp

#include <memory>
#include <map>
#include <vector>

#include <windows.h>
#include <propsys.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <xmllite.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "xmllite.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\Functions.h"
#include "PluginManager.h"

//---------------------------------------------------------------------------//

#define MSG_01_E TEXT("Cannot find plugin directory.\nWould you like to create it?")
#define MSG_01_J TEXT("plugins フォルダが見つかりません。\n作成しますか？")
#define MSG_01 MSG_01_J

#define NAME1 TEXT("PluginManager")
#define NAME2 TEXT("PluginContainer")

#define MAX_PLUGIN_COUNT 256

//---------------------------------------------------------------------------//

inline bool operator<(const GUID& lhs, const GUID& rhs)
{
    return memcmp(&lhs, &rhs, sizeof(IID)) < 0;
}

//---------------------------------------------------------------------------//

struct PluginContainer::Impl
{
    Impl(LPCWSTR file_path, size_t index);
    ~Impl();
    void __stdcall Reset();
    void __stdcall GetProperty();

    size_t            index;
    LPWSTR            copyright;
    LPWSTR            description;
    LPWSTR            name;
    HMODULE           hModule;
    DLLCANUNLOADNOW   DllCanUnloadNow;
    DLLGETPROPERTY    DllGetProperty;
    DLLGETCLASSOBJECT DllGetClassObject;
    CLSID             clsid;
    WCHAR             file_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

PluginContainer::Impl::Impl(LPCWSTR file_path, size_t index)
{
    DebugPrintLn(NAME2 TEXT("::Impl::Constructor() begin"));

    this->index       = index;
    ::StringCchPrintf(this->file_path, MAX_PATH, file_path);

    copyright         = nullptr;
    description       = nullptr;
    name              = nullptr;
    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllGetProperty    = nullptr;
    DllGetClassObject = nullptr;
    clsid             = CLSID_NULL;

    DebugPrintLn(NAME2 TEXT("::Impl::Constructor() end"));
}

//---------------------------------------------------------------------------//

PluginContainer::Impl::~Impl()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Destructor() begin"));

    this->Reset();

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

void __stdcall PluginContainer::Impl::Reset()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Reset() begin"));

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

    DebugPrintLn(NAME2 TEXT("::Impl::Reset() end"));
}

//---------------------------------------------------------------------------//

void __stdcall PluginContainer::Impl::GetProperty()
{
    HRESULT hr;
    IPropertyStore* ps = nullptr;
    PROPERTYKEY key;
    PROPVARIANT pv;

    DebugPrintLn(NAME2 TEXT("::Impl::GetProperty() begin"));

    // IPropertyStore オブジェクトを取得
    hr = DllGetProperty(index, &ps);
    if ( FAILED(hr) || nullptr == ps )
    {
        DebugPrintLn(TEXT("No object at index == %d"), index);
        return;
    }

    // クラスIDを取得
    DebugPrintLn(TEXT("Getting ClassID..."));
    key.fmtid = PKEY_CubeMelon_GetProperty;
    key.pid   = CUBEMELON_PID_CLSID;
    ::PropVariantInit(&pv);
    hr = ps->GetValue(key, &pv);
    if ( FAILED(hr) || pv.vt != VT_CLSID )
    {
        DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
        return;
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

    // プラグイン名を取得
    DebugPrintLn(TEXT("Getting plugin name..."));
    key.fmtid = PKEY_CubeMelon_GetProperty;
    key.pid   = CUBEMELON_PID_NAME;
    ::PropVariantInit(&pv);
    hr = ps->GetValue(key, &pv);
    if ( FAILED(hr) || pv.vt != VT_LPWSTR )
    {
        DebugPrintLn(TEXT("IPropertyStore::GetValue() failed"));
        return;
    }

    auto str_count = sizeof(nullptr) + wcslen(pv.pwszVal);
    name = (LPWSTR)::CoTaskMemAlloc(sizeof(WCHAR) * (str_count));
    ::StringCchPrintf(name, str_count, pv.pwszVal);
    ::PropVariantClear(&pv); // なるべくすぐに pv を解放
    DebugPrintLn(name);
    DebugPrintLn(TEXT("Got plugin properties"));

    ps->Release();
    ps = nullptr;

    DebugPrintLn(NAME2 TEXT("::Impl::GetProperty() end"));
}

//---------------------------------------------------------------------------//

PluginContainer::PluginContainer(LPCWSTR file_path, size_t index)
{
    DebugPrintLn(NAME2 TEXT("::Constructor() begin"));

    pimpl = new Impl(file_path, index);

    m_cRef = 0;

    DebugPrintLn(NAME2 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

PluginContainer::~PluginContainer()
{
    DebugPrintLn(NAME2 TEXT("::Destructor() begin"));

    this->Free();

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME2 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginContainer::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME2 TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPluginContainer) )
    {
        *ppvObject = static_cast<IPluginContainer*>(this);
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

ULONG __stdcall PluginContainer::AddRef()
{
    DebugPrintLn(NAME2 TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME2 TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall PluginContainer::Release()
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

REFCLSID __stdcall PluginContainer::ClassID() const
{
    return pimpl->clsid;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall PluginContainer::Copyright() const
{
    return pimpl->copyright;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall PluginContainer::Description() const
{
    return pimpl->description;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall PluginContainer::FilePath() const
{
    return pimpl->file_path;
}

//---------------------------------------------------------------------------//

size_t __stdcall PluginContainer::Index() const
{
    return pimpl->index;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall PluginContainer::Name() const
{
    return pimpl->name;
}

//---------------------------------------------------------------------------//

VersionInfo* __stdcall PluginContainer::Version() const
{
    return nullptr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginContainer::Load()
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
            throw TEXT("Failed to load plugin");
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

        // プラグイン情報を取得
        pimpl->GetProperty();
        if ( IsEqualCLSID(pimpl->clsid, CLSID_NULL) )
        {
            throw TEXT("DllGetProperty() failed");
        }
    }
    catch (LPCWSTR msg)
    {
        DebugPrintLn(msg);
        msg = nullptr;

        ///pimpl->Reset();

        return E_FAIL;
    }

    DebugPrintLn(NAME2 TEXT("::Load() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginContainer::Free()
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
        DebugPrintLn(TEXT("Plugin is locked"));
        return E_FAIL;
    }

    pimpl->Reset();

    DebugPrintLn(NAME2 TEXT("::Free() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginContainer::GetProperty(IPropertyStore** ps)
{
    return pimpl->DllGetProperty(pimpl->index, ps);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginContainer::GetClassObject(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME2 TEXT("::GetClassObject() begin"));

    if ( nullptr == pimpl->hModule )
    {
        auto hr = this->Load();
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    DebugPrintLn(NAME2 TEXT("::GetClassObject() end"));

    return pimpl->DllGetClassObject(pimpl->clsid, riid, ppvObject);
}

//---------------------------------------------------------------------------//

typedef std::map<CLSID, std::unique_ptr<PluginContainer>> PluginMap;

struct PluginManager::Impl
{
    Impl();
    ~Impl();
    void __stdcall Reset();

    void __stdcall ScanDirectory(LPCWSTR dir_path);
    void __stdcall RegisterPlugin(LPCWSTR plugin_path);

    size_t              count;
    ::PluginContainer** plugins;
    PluginMap           pmap;
    WCHAR               dir_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

PluginManager::Impl::Impl()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Constructor() begin"));

    count = 0;
    plugins = nullptr;
    dir_path[0] = '\0';

    DebugPrintLn(NAME1 TEXT("::Impl::Constructor() end"));
}

//---------------------------------------------------------------------------//

PluginManager::Impl::~Impl()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Destructor() begin"));

    this->Reset();

    DebugPrintLn(NAME1 TEXT("::Impl::Destructor() end"));
}

//---------------------------------------------------------------------------//

void __stdcall PluginManager::Impl::Reset()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Reset() begin"));

    pmap.clear();

    dir_path[0] = '\0';

    if ( plugins )
    {
        delete[] plugins;
        plugins = nullptr;
    }

    count = 0;

    DebugPrintLn(NAME1 TEXT("::Impl::Reset() end"));
}

//---------------------------------------------------------------------------//

void __stdcall PluginManager::Impl::ScanDirectory(LPCWSTR dir_path)
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
        auto ret = ::MessageBox(nullptr, MSG_01, nullptr, MB_YESNO | MB_DEFBUTTON2);
        if ( ret == IDYES )
        {
            DebugPrintLn(TEXT("Creating plugins directory..."));
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
        ::StringCchPrintf(path, MAX_PATH, TEXT("%s\\%s"), dir_path, fd.cFileName);

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
                // プラグインデータベースに記憶
                RegisterPlugin(path);
            }
        }

        DebugPrintLn(TEXT("Next file"));
    }
    while ( ::FindNextFile(hFindFile, &fd) == TRUE );

    ::FindClose( hFindFile );

    DebugPrintLn(NAME1 TEXT("::ScanDirectory() end"));
}

//---------------------------------------------------------------------------//

void __stdcall PluginManager::Impl::RegisterPlugin(LPCWSTR plugin_path)
{
    DebugPrintLn(NAME1 TEXT("::RegisterPlugin() begin"));
    DebugPrintLn(plugin_path);

    ::PluginContainer* pc = nullptr;
    CLSID clsid = CLSID_NULL;

    // DLLファイルに含まれているクラスIDを全て取得
    for ( size_t index = 0; index < MAX_PLUGIN_COUNT ; ++index )
    {
        DebugPrintLn(TEXT("Searching ClassIDs: index == %d"), index);

        // プラグイン情報を管理するオブジェクトを生成
        pc = new ::PluginContainer(plugin_path, index);
        auto hr = pc->Load();
        if ( FAILED(hr) )
        {
            DebugPrintLn(TEXT("Disposing plugin container..."));

            delete pc;
            pc = nullptr;

            DebugPrintLn(TEXT("Disposed"));

            break;
        }

        // プラグインのクラスIDを取得
        clsid = pc->ClassID();

        // 同じクラスIDがすでに登録されていないか確認
        DebugPrintLn(TEXT("Checking ClassID..."));
        {
            auto it = pmap.find(clsid);
            if ( it != pmap.end() )
            {
                DebugPrintLn(TEXT("Duplicate ClassID"));
                continue;
            }
        }
        DebugPrintLn(TEXT("Checked ClassID: no duplicate"));

        // プラグイン情報を本体のデータベースに記憶
        DebugPrintLn(TEXT("Registering plugin..."));
        {
            pmap.emplace
            (
                std::make_pair(clsid, std::unique_ptr<::PluginContainer>(pc))
            );
        }
        DebugPrintLn(TEXT("Registered plugin"));
    }
    DebugPrintLn(TEXT("Searched ClassIDs"));

    DebugPrintLn(NAME1 TEXT("::RegisterPlugin() end"));
}

//---------------------------------------------------------------------------//

PluginManager::PluginManager(LPCWSTR dir_path)
{
    DebugPrintLn(NAME1 TEXT("::Constructor() begin"));

    pimpl = new Impl;
    ::StringCchPrintf(pimpl->dir_path, MAX_PATH, dir_path);

    m_cRef = 0;

    this->AddRef();

    DebugPrintLn(NAME1 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

PluginManager::~PluginManager()
{
    DebugPrintLn(NAME1 TEXT("::Destructor() begin"));

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME1 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginManager::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME1 TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IPluginManager) )
    {
        *ppvObject = static_cast<IPluginManager*>(this);
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

ULONG __stdcall PluginManager::AddRef()
{
    DebugPrintLn(NAME1 TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME1 TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall PluginManager::Release()
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

LPCWSTR __stdcall PluginManager::DirectoryPath() const
{
    return pimpl->dir_path;
}

//---------------------------------------------------------------------------//

size_t __stdcall PluginManager::PluginCount() const
{
    return pimpl->count;
}

//---------------------------------------------------------------------------//

IPluginContainer* __stdcall PluginManager::PluginContainer(size_t index) const
{
    if ( index < pimpl->count )
    {
        return static_cast<IPluginContainer*>(pimpl->plugins[index]);
    }
    else
    {
        return nullptr;
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginManager::LoadAll()
{
    DebugPrintLn(NAME1 TEXT("::LoadAll() begin"));

    pimpl->ScanDirectory(pimpl->dir_path);

    pimpl->count = pimpl->pmap.size();
    if ( pimpl->count < 1 )
    {
        DebugPrintLn(TEXT("No plugin found"));
    }

    pimpl->plugins = new ::PluginContainer*[pimpl->count + 1]; // 0でも空の配列を作る

    auto it  = pimpl->pmap.begin();
    auto end = pimpl->pmap.end();
    for ( size_t i = 0; it != end; ++i, ++it )
    {
        pimpl->plugins[i] = it->second.get();
    }
    pimpl->plugins[pimpl->count] = nullptr; // 配列はNULL終端

    DebugPrintLn(NAME1 TEXT("::LoadAll() end"));

    return pimpl->count ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginManager::FreeAll()
{
    DebugPrintLn(NAME1 TEXT("::FreeAll() begin"));

    HRESULT hr;
    size_t freed = 0;

    auto count = pimpl->count;
    for ( size_t i = 0; i < count; ++i )
    {
        hr = pimpl->plugins[i]->Free();
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

// PluginManager.cpp