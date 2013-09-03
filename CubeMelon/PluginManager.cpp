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
#include "..\include\PluginManager.h"

//---------------------------------------------------------------------------//

#define MSG_01_E TEXT("Cannot find plugin directory.\nWould you like to create it?")
#define MSG_01_J TEXT("plugins フォルダが見つかりません。\n作成しますか？")
#define MSG_01 MSG_01_J

#define NAME1 TEXT("PluginManager")
#define NAME2 TEXT("PluginInstance")

#define MAX_PLUGIN_INSTANCE 255

//---------------------------------------------------------------------------//

inline bool operator<(const GUID& lhs, const GUID& rhs)
{
    return memcmp(&lhs, &rhs, sizeof(IID)) < 0;
}

//---------------------------------------------------------------------------//

struct PluginInstance::Impl
{
    Impl();
    ~Impl();
    void __stdcall Reset();
    void __stdcall GetProperty();

    size_t index;
    HMODULE hModule;
    DLLCANUNLOADNOW   DllCanUnloadNow;
    DLLGETPROPERTY    DllGetProperty;
    DLLGETCLASSOBJECT DllGetClassObject;
    CLSID clsid;
    WCHAR name[MAX_PATH];
    WCHAR file_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

PluginInstance::Impl::Impl()
{
    index             = 0;
    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllGetProperty    = nullptr;
    DllGetClassObject = nullptr;
    clsid             = CLSID_NULL;
    name[0]           = '\0';
    file_path[0]      = '\0';
}

//---------------------------------------------------------------------------//

PluginInstance::Impl::~Impl()
{
    this->Reset();
}

//---------------------------------------------------------------------------//

void __stdcall PluginInstance::Impl::Reset()
{
    if ( hModule )
    {
        ::FreeLibrary(hModule);
    }

    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllGetProperty    = nullptr;
    DllGetClassObject = nullptr;
}

//---------------------------------------------------------------------------//

void __stdcall PluginInstance::Impl::GetProperty()
{
    HRESULT hr;
    IPropertyStore* ps = nullptr;
    PROPERTYKEY key;
    PROPVARIANT pv;

    DebugPrintLn(NAME2 TEXT("::Impl::GetProperty() begin"));

    if ( name[0] != '\0' )
    {
        DebugPrintLn(TEXT("\tAlready got"));
        return;
    }

    // IPropertyStore オブジェクトを取得
    hr = DllGetProperty(index, &ps);
    if ( FAILED(hr) || nullptr == ps )
    {
        DebugPrintLn(TEXT("\tNo object at index == %d"), index);
        return;
    }

    // クラスIDを取得
    DebugPrintLn(TEXT("\tGetting ClassID..."));
    key.fmtid = PKEY_CubeMelon_GetProperty;
    key.pid   = CUBEMELON_PID_CLSID;
    ::PropVariantInit(&pv);
    hr = ps->GetValue(key, &pv);
    if ( FAILED(hr) || pv.vt != VT_CLSID )
    {
        DebugPrintLn(TEXT("\tIPropertyStore::GetValue() failed"));
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
    DebugPrintLn(TEXT("\tGetting plugin name..."));
    key.fmtid = PKEY_CubeMelon_GetProperty;
    key.pid   = CUBEMELON_PID_NAME;
    ::PropVariantInit(&pv);
    hr = ps->GetValue(key, &pv);
    if ( FAILED(hr) || pv.vt != VT_LPWSTR )
    {
        DebugPrintLn(TEXT("\tIPropertyStore::GetValue() failed"));
        return;
    }

    ::StringCchPrintf(name, MAX_PATH, pv.pwszVal);
    ::PropVariantClear(&pv); // なるべくすぐに pv を解放
    DebugPrintLn(name);
    DebugPrintLn(TEXT("\tGot plugin properties"));

    ps->Release();
    ps = nullptr;

    DebugPrintLn(NAME2 TEXT("::Impl::GetProperty() end"));
}

//---------------------------------------------------------------------------//

PluginInstance::PluginInstance(LPCWSTR file_path, size_t index)
{
    DebugPrintLn(NAME2 TEXT("::Constructor() begin"));

    pimpl = new Impl;
    ::StringCchPrintf(pimpl->file_path, MAX_PATH, file_path);
    pimpl->index = index;

    DebugPrintLn(NAME2 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

PluginInstance::~PluginInstance()
{
    DebugPrintLn(NAME2 TEXT("::Destructor() begin"));

    this->Free();

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME2 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall PluginInstance::Name() const
{
    return pimpl->name;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall PluginInstance::FilePath() const
{
    return pimpl->file_path;
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall PluginInstance::ClassID() const
{
    return pimpl->clsid;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginInstance::Load()
{
    DebugPrintLn(NAME2 TEXT("::Load() begin"));
    DebugPrintLn(pimpl->file_path);

    if ( pimpl->hModule )
    {
        DebugPrintLn(TEXT("\tAlready loaded"));
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
            throw TEXT("\tFailed to load plugin");
        }

        // 公開関数のアドレスを取得
        pimpl->DllCanUnloadNow = (DLLCANUNLOADNOW)::GetProcAddress
        (
            pimpl->hModule, "DllCanUnloadNow"
        );
        if ( nullptr == pimpl->DllCanUnloadNow )
        {
            throw TEXT("\tDllCanUnloadNow() is not found");
        }

        pimpl->DllGetProperty = (DLLGETPROPERTY)::GetProcAddress
        (
            pimpl->hModule, "DllGetProperty"
        );
        if ( nullptr == pimpl->DllGetProperty )
        {
            throw TEXT("\tDllGetProperty() is not found");
        }

        pimpl->DllGetClassObject = (DLLGETCLASSOBJECT)::GetProcAddress
        (
            pimpl->hModule, "DllGetClassObject"
        );
        if ( nullptr == pimpl->DllGetClassObject )
        {
            throw TEXT("\tDllGetClassObject() is not found");
        }

        // プラグイン情報を取得
        pimpl->GetProperty();
        if ( pimpl->name[0] == '\0' )
        {
            throw TEXT("\tDllGetProperty() failed");
        }
    }
    catch (LPCWSTR msg)
    {
        DebugPrintLn(msg);
        msg = nullptr;

        pimpl->Reset();

        return E_FAIL;
    }

    DebugPrintLn(NAME2 TEXT("::Load() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginInstance::Free()
{
    DebugPrintLn(NAME2 TEXT("::Free() begin"));
    DebugPrintLn(pimpl->file_path);

    if ( nullptr == pimpl->hModule )
    {
        DebugPrintLn(TEXT("\tAlready Freed"));
        return S_FALSE;
    }

    auto hr = pimpl->DllCanUnloadNow();
    if ( S_OK != hr )
    {
        DebugPrintLn(TEXT("\tPlugin is locked"));
        return E_FAIL;
    }

    pimpl->Reset();

    DebugPrintLn(NAME2 TEXT("::Free() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginInstance::GetClassObject(REFIID riid, void** ppvObject)
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

typedef std::map<CLSID, std::unique_ptr<PluginInstance>> PluginMap;

struct PluginManager::Impl
{
    Impl();
    ~Impl();
    void __stdcall Reset();

    void __stdcall ScanDirectory(LPCWSTR dir_path);
    void __stdcall RegisterPlugin(LPCWSTR plugin_path);

    PluginMap pmap;
    PluginInstance** plugins;
};

//---------------------------------------------------------------------------//

PluginManager::Impl::Impl()
{
    plugins = nullptr;
}

//---------------------------------------------------------------------------//

PluginManager::Impl::~Impl()
{
    this->Reset();
}

//---------------------------------------------------------------------------//

void __stdcall PluginManager::Impl::Reset()
{
    delete[] plugins;
    plugins = nullptr;
}

//---------------------------------------------------------------------------//

void __stdcall PluginManager::Impl::ScanDirectory(LPCWSTR dir_path)
{
    DebugPrintLn(NAME1 TEXT("::ScanDirectory() begin"));

    WCHAR path[MAX_PATH];
    ::StringCchPrintf(path, MAX_PATH, TEXT("%s\\*"), dir_path);
    DebugPrintLn(TEXT("\tScanning..."));
    DebugPrintLn(path);

    // フォルダ内にあるものを列挙する
    WIN32_FIND_DATA fd = { 0 };
    auto hFindFile = ::FindFirstFile(path, &fd);
    if ( hFindFile == INVALID_HANDLE_VALUE )
    {
        auto ret = ::MessageBox(nullptr, MSG_01, nullptr, MB_YESNO | MB_DEFBUTTON2);
        if ( ret == IDYES )
        {
            DebugPrintLn(TEXT("\tCreating plugins directory..."));
            ::CreateDirectory(dir_path, nullptr);
            DebugPrintLn(TEXT("\tCreated."));
        }
        return;
    }

    do
    {
        DebugPrintLn(TEXT("\tChecking file attribute..."));
        DebugPrintLn(fd.cFileName);

        // ファイル名がピリオドで始まっているものは飛ばす
        if ( fd.cFileName && fd.cFileName[0] == '.' )
        {
            DebugPrintLn(TEXT("\tThis is a dot file"));
            continue;
        }

        // 隠し属性を持つものは飛ばす
        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
        {
            DebugPrintLn(TEXT("\tThis is a hidden file"));
            continue;
        }

        DebugPrintLn(TEXT("\tChecked file attribute"));

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

    PluginInstance* pi = nullptr;
    CLSID clsid = CLSID_NULL;

    // DLLファイルに含まれているクラスIDを全て取得
    for ( size_t index = 0; index < MAX_PLUGIN_INSTANCE ; ++index )
    {
        DebugPrintLn(TEXT("\tSearching ClassIDs: index == %d"), index);

        // プラグイン情報を管理するオブジェクトを生成
        pi = new PluginInstance(plugin_path, index);
        auto hr = pi->Load();
        if ( FAILED(hr) )
        {
            delete pi;
            pi = nullptr;
            break;
        }
        ///pi->Free();

        // プラグインのクラスIDを取得
        clsid = pi->ClassID();

        // 同じクラスIDがすでに登録されていないか確認
        DebugPrintLn(TEXT("\tChecking ClassID..."));
        {
            auto it = pmap.find(clsid);
            if ( it != pmap.end() )
            {
                DebugPrintLn(TEXT("\tDuplicate ClassID"));
                continue;
            }
        }
        DebugPrintLn(TEXT("\tChecked ClassID: no duplicate"));

        // プラグイン情報を本体のデータベースに記憶
        DebugPrintLn(TEXT("\tRegistering plugin..."));
        {
            pmap.emplace
            (
                std::make_pair(clsid, std::unique_ptr<PluginInstance>(pi))
            );
        }
        DebugPrintLn(TEXT("\tRegistered plugin"));
    }
    DebugPrintLn(TEXT("\tSearched ClassIDs"));

    DebugPrintLn(NAME1 TEXT("::RegisterPlugin() end"));
}

//---------------------------------------------------------------------------//

PluginManager::PluginManager()
{
    DebugPrintLn(NAME1 TEXT("::Constructor() begin"));

    pimpl = new Impl;

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

size_t __stdcall PluginManager::PluginCount() const
{
    return pimpl->pmap.size();
}

//---------------------------------------------------------------------------//

PluginInstance** __stdcall PluginManager::AllPlugins() const
{
    return pimpl->plugins;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginManager::LoadAll()
{
    DebugPrintLn(NAME1 TEXT("::LoadAll() begin"));

    WCHAR dir_path[MAX_PATH];
    ::GetModuleFileName(nullptr, dir_path, MAX_PATH);
    ::PathRemoveFileSpec(dir_path);
    ::PathAppend(dir_path, TEXT("\\plugins"));

    pimpl->ScanDirectory(dir_path);

    auto count = pimpl->pmap.size();
    if ( count < 1 )
    {
        DebugPrintLn(TEXT("\tNo plugin found"));
    }

    pimpl->plugins = new PluginInstance*[count + 1]; // 0でも空の配列を作る

    auto it  = pimpl->pmap.begin();
    auto end = pimpl->pmap.end();
    for ( size_t i = 0; it != end; ++i, ++it )
    {
        pimpl->plugins[i] = it->second.get();
    }
    pimpl->plugins[count] = nullptr; // 配列はNULL終端

    DebugPrintLn(NAME1 TEXT("::LoadAll() end"));

    return count ? S_OK : E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginManager::FreeAll()
{
    DebugPrintLn(NAME1 TEXT("::FreeAll() begin"));

    HRESULT hr;
    size_t freed = 0;

    auto count = pimpl->pmap.size();
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
        DebugPrintLn(TEXT("\tFreed nothing"));
        hr = E_FAIL;
    }
    else if ( freed == count )
    {
        DebugPrintLn(TEXT("\tFreed all"));
        hr = S_OK;
    }
    else
    {
        DebugPrintLn(TEXT("\tNot freed all"));
        hr = S_FALSE;
    }

    DebugPrintLn(NAME1 TEXT("::FreeAll() end"));

    return hr;
}

//---------------------------------------------------------------------------//

// PluginManager.cpp