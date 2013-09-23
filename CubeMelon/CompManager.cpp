// CompManager.cpp

#include <map>

#include <windows.h>
#include <strsafe.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "..\include\ComPtr.h"
#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Functions.h"
#include "..\include\Interfaces.h"

#include "CompManager.h"

//---------------------------------------------------------------------------//

#define MSG_01_E TEXT("Cannot find component directory.\nWould you like to create it?")
#define MSG_01_J TEXT("components フォルダが見つかりません。\n作成しますか？")
#define MSG_01 MSG_01_J

#define NAME1 TEXT("CubeMelon.CompManager")
#define NAME2 TEXT("CubeMelon.CompAdapter")

#define MAX_COMPONENT_COUNT 256

//---------------------------------------------------------------------------//

inline bool operator<(const GUID& lhs, const GUID& rhs)
{
    return memcmp(&lhs, &rhs, sizeof(IID)) < 0;
}

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct CompAdapter::Impl
{
    Impl();
    ~Impl();
    HRESULT __stdcall Free();
    HRESULT __stdcall GetTypicalProperties();

    size_t            index;
    HMODULE           hModule;
    DLLCANUNLOADNOW   DllCanUnloadNow;
    DLLCONFIGURE      DllConfigure;
    DLLGETCLASSOBJECT DllGetClassObject;
    DLLGETPROPMANAGER DllGetPropManager;
    IProperty*        clsid;
    IProperty*        name;
    IProperty*        copyright;
    IProperty*        description;
    IProperty*        version;
    IPropManager*     prop_mgr;
    WCHAR             file_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

CompAdapter::Impl::Impl()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Constructor() begin"));

    index             = 0;
    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllConfigure      = nullptr;
    DllGetClassObject = nullptr;
    DllGetPropManager = nullptr;
    clsid             = nullptr;
    name              = nullptr;
    description       = nullptr;
    copyright         = nullptr;
    version           = nullptr;
    prop_mgr          = nullptr;
    file_path[0]      = '\0';

    DebugPrintLn(NAME2 TEXT("::Impl::Constructor() end"));
}

//---------------------------------------------------------------------------//

CompAdapter::Impl::~Impl()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Destructor() begin"));

    this->Free();

    DebugPrintLn(NAME2 TEXT("::Impl::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::Impl::Free()
{
    DebugPrintLn(NAME2 TEXT("::Impl::Free() begin"));

    DebugPrintLn(file_path);

    if ( clsid )
    {
        clsid->Release();
        clsid = nullptr;
    }
    if ( name )
    {
        name->Release();
        name = nullptr;
    }
    if ( description )
    {
        description->Release();
        description = nullptr;
    }
    if ( copyright )
    {
        copyright->Release();
        copyright = nullptr;
    }
    if ( version )
    {
        version->Release();
        version = nullptr;
    }
    if ( prop_mgr )
    {
        prop_mgr->Release();
        prop_mgr = nullptr;
    }

    if ( nullptr == hModule )
    {
        DebugPrintLn(TEXT(". Already Freed"));
        DebugPrintLn(NAME2 TEXT("::Impl::Free() end"));
        return S_FALSE;
    }

    auto hr = DllCanUnloadNow();
    if ( S_OK != hr )
    {
        DebugPrintLn(TEXT(". Module is locked"));
        DebugPrintLn(NAME2 TEXT("::Impl::Free() end"));
        return E_FAIL;
    }

    DebugPrintLn(TEXT(". Freeing DLL..."));
    {
        ::FreeLibrary(hModule);
    }
    DebugPrintLn(TEXT(". Freed DLL"));

    hModule           = nullptr;
    DllCanUnloadNow   = nullptr;
    DllGetPropManager = nullptr;
    DllGetClassObject = nullptr;

    DebugPrintLn(NAME2 TEXT("::Impl::Free() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::Impl::GetTypicalProperties()
{
    HRESULT hr;

    DebugPrintLn(NAME2 TEXT("::Impl::GetTypicalProperties() begin"));

    // IPropManager オブジェクトを取得
    if ( nullptr == prop_mgr )
    {
        hr = this->DllGetPropManager(this->index, &this->prop_mgr);
    }
    if ( nullptr == this->prop_mgr )
    {
        DebugPrintLn(TEXT(". No object at index == %d"), this->index);
        DebugPrintLn(NAME2 TEXT("::Impl::GetTypicalProperties() end"));
        return hr;
    }

    IProperty* prop = nullptr;

    this->prop_mgr->GetByName(TEXT("ClassID"), &prop);
    if ( prop )
    {
        DebugPrintLn(TEXT(". Getting ClassID..."));

        this->clsid = prop;

        auto id = this->clsid->Data()->clsid;
        DebugPrintLn
        (
            TEXT("{ %00000000X-%0000X-%0000X-%00X%00X-%00X%00X%00X%00X%00X%00X }"),
            id.Data1, id.Data2, id.Data3,
            id.Data4[0], id.Data4[1], id.Data4[2], id.Data4[3],
            id.Data4[4], id.Data4[5], id.Data4[6], id.Data4[7]
        );
    }

    this->prop_mgr->GetByName(TEXT("Name"), &prop);
    if ( prop )
    {
        DebugPrintLn(TEXT(". Getting component name..."));

        this->name = prop;

        DebugPrintLn(this->name->Data()->wstring);
    }

    this->prop_mgr->GetByName(TEXT("Description"), &prop);
    if ( prop )
    {
        DebugPrintLn(TEXT(". Getting component description..."));

        this->description = prop;

        DebugPrintLn(this->description->Data()->wstring);
    }

    this->prop_mgr->GetByName(TEXT("Copyright"), &prop);
    if ( prop )
    {
        DebugPrintLn(TEXT(". Getting component copyright information..."));

        this->copyright = prop;

        DebugPrintLn(this->copyright->Data()->wstring);
    }

    this->prop_mgr->GetByName(TEXT("VersionInfo"), &prop);
    if ( prop )
    {
        DebugPrintLn(TEXT(". Getting version information..."));

        this->version = prop;

        auto ver = this->version->Data()->version;
        DebugPrintLn
        (
            TEXT("%d.%d.%d.%d"),
            ver->major, ver->minor, ver->revision, ver->stage
        );
    }

    prop = nullptr;

    if ( nullptr == this->clsid )
    {
        DebugPrintLn(TEXT(". Failed to get ClassID"));
        return E_FAIL;
    }

    DebugPrintLn(NAME2 TEXT("::Impl::GetTypicalProperties() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

CompAdapter::CompAdapter()
{
    DebugPrintLn(NAME2 TEXT("::Constructor() begin"));

    pimpl = new Impl();

    m_cRef = 0;

    this->AddRef();

    DebugPrintLn(NAME2 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

CompAdapter::~CompAdapter()
{
    DebugPrintLn(NAME2 TEXT("::Destructor() begin"));

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME2 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME2 TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICompAdapter) )
    {
        *ppvObject = static_cast<ICompAdapter*>(this);
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

ULONG __stdcall CompAdapter::AddRef()
{
    DebugPrintLn(NAME2 TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME2 TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall CompAdapter::Release()
{
    DebugPrintLn(NAME2 TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT(". Deleting..."));
        delete this;
        DebugPrintLn(TEXT(". Deleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME2 TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall CompAdapter::ClassID() const
{
    return pimpl->clsid ? pimpl->clsid->Data()->clsid : CLSID_NULL;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall CompAdapter::Copyright() const
{
    return pimpl->copyright ? pimpl->copyright->Data()->wstring : nullptr;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall CompAdapter::Description() const
{
    return pimpl->description ? pimpl->description->Data()->wstring : nullptr;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall CompAdapter::FilePath() const
{
    return pimpl->file_path;
}

//---------------------------------------------------------------------------//

size_t __stdcall CompAdapter::Index() const
{
    return pimpl->index;
}

//---------------------------------------------------------------------------//

IPropManager* __stdcall CompAdapter::PropManager() const
{
    if ( pimpl->prop_mgr )
    {
        pimpl->prop_mgr->AddRef();
    }

    return pimpl->prop_mgr;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall CompAdapter::Name() const
{
    return pimpl->name ? pimpl->name->Data()->wstring : nullptr;
}

//---------------------------------------------------------------------------//

VersionInfo* __stdcall CompAdapter::Version() const
{
    return pimpl->version ? pimpl->version->Data()->version : nullptr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::Load(LPCWSTR file_path, size_t index)
{
    DebugPrintLn(NAME2 TEXT("::Load() begin"));

    ::StringCchPrintf(pimpl->file_path, MAX_PATH, file_path);
    pimpl->index = index;

    DebugPrintLn(pimpl->file_path);

    if ( pimpl->hModule )
    {
        DebugPrintLn(TEXT(". Already loaded"));
        DebugPrintLn(NAME2 TEXT("::Load() end"));
        return S_FALSE;
    }

    // DLLの読み込み
    pimpl->hModule = ::LoadLibraryEx
    (
        pimpl->file_path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );
    if ( nullptr == pimpl->hModule )
    {
        goto FREE_DLL;
    }

    // 公開関数のアドレスを取得
    pimpl->DllCanUnloadNow = (DLLCANUNLOADNOW)::GetProcAddress
    (
        pimpl->hModule, "DllCanUnloadNow"
    );
    if ( nullptr == pimpl->DllCanUnloadNow )
    {
        goto FREE_DLL;
    }

    pimpl->DllConfigure = (DLLCONFIGURE)::GetProcAddress
    (
        pimpl->hModule, "DllConfigure"
    );
    if ( nullptr == pimpl->DllConfigure )
    {
        goto FREE_DLL;
    }

    pimpl->DllGetClassObject = (DLLGETCLASSOBJECT)::GetProcAddress
    (
        pimpl->hModule, "DllGetClassObject"
    );
    if ( nullptr == pimpl->DllGetClassObject )
    {
        goto FREE_DLL;
    }

    pimpl->DllGetPropManager = (DLLGETPROPMANAGER)::GetProcAddress
    (
        pimpl->hModule, "DllGetPropManager"
    );
    if ( nullptr == pimpl->DllGetPropManager )
    {
        goto FREE_DLL;
    }

    // ClassIDなど、主なコンポーネント情報を取得
    auto hr = pimpl->GetTypicalProperties();
    if ( FAILED(hr) )
    {
        DebugPrintLn(NAME2 TEXT("::Load() end"));
        return hr;
    }

    DebugPrintLn(NAME2 TEXT("::Load() end"));

    return S_OK;

FREE_DLL:
    DebugPrintLn(TEXT(". This dll file does not have full implementation as a CubeMelon component"));
    DebugPrintLn(NAME2 TEXT("::Load() end"));
    pimpl->Free();

    return E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::Free()
{
    DebugPrintLn(NAME2 TEXT("::Free() begin"));

    auto hr = pimpl->Free();

    DebugPrintLn(NAME2 TEXT("::Free() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::Configure(HWND hwndParent)
{
    return pimpl->DllConfigure(hwndParent);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompAdapter::CreateInstance
(
    IComponent* owner, REFIID riid, void** ppvObject
)
{
    HRESULT hr;

    DebugPrintLn(NAME2 TEXT("::CreateInstance() begin"));

    if ( nullptr == pimpl->hModule )
    {
        DebugPrintLn(TEXT(". Module file is not yet open"));
        DebugPrintLn(NAME2 TEXT("::CreateInstance() end"));
        return E_FAIL;
    }

    IClassFactory* factory = nullptr;
    hr = pimpl->DllGetClassObject
    (
        this->ClassID(), IID_IClassFactory, (void**)&factory
    );
    if ( nullptr == factory )
    {
        DebugPrintLn(TEXT(". Could not get factory object"));
        DebugPrintLn(NAME2 TEXT("::CreateInstance() end"));
        return hr;
    }

    hr = factory->CreateInstance
    (
        owner, riid, ppvObject
    );
    factory->Release();
    factory = nullptr;
    if ( nullptr == *ppvObject )
    {
        DebugPrintLn(TEXT(". Could not create new instance"));
        DebugPrintLn(NAME2 TEXT("::CreateInstance() end"));
        return hr;
    }

    DebugPrintLn(NAME2 TEXT("::CreateInstance() end"));

    return hr;
}

//---------------------------------------------------------------------------//

typedef std::map<CLSID, ComPtr<ICompAdapter>> ComponentMap;

struct CompManager::Impl
{
    Impl();
    ~Impl();
    void __stdcall Reset();

    void __stdcall ScanDirectory(LPCWSTR dir_path);
    void __stdcall RegisterComponent(LPCWSTR comp_path);

    size_t         count;
    ICompAdapter** comps;
    ComponentMap   cmap;
    WCHAR          dir_path[MAX_PATH];
};

//---------------------------------------------------------------------------//

CompManager::Impl::Impl()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Constructor() begin"));

    count       = 0;
    comps       = nullptr;
    dir_path[0] = '\0';

    DebugPrintLn(NAME1 TEXT("::Impl::Constructor() end"));
}

//---------------------------------------------------------------------------//

CompManager::Impl::~Impl()
{
    DebugPrintLn(NAME1 TEXT("::Impl::Destructor() begin"));

    this->Reset();

    DebugPrintLn(NAME1 TEXT("::Impl::Destructor() end"));
}

//---------------------------------------------------------------------------//

void __stdcall CompManager::Impl::Reset()
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

void __stdcall CompManager::Impl::ScanDirectory(LPCWSTR dir_path)
{
    DebugPrintLn(NAME1 TEXT("::ScanDirectory() begin"));

    WCHAR path[MAX_PATH];
    ::StringCchPrintf(path, MAX_PATH, TEXT("%s\\*"), dir_path);
    DebugPrintLn(TEXT(". Scanning..."));
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
            DebugPrintLn(TEXT(". . Creating components directory..."));
            ::CreateDirectory(dir_path, nullptr);
            DebugPrintLn(TEXT(". . Created components directory"));
        }
        return;
    }

    do
    {
        DebugPrintLn(TEXT(". Checking file attribute..."));
        DebugPrintLn(fd.cFileName);

        // ファイル名がピリオドで始まっているものは飛ばす
        if ( fd.cFileName && fd.cFileName[0] == '.' )
        {
            DebugPrintLn(TEXT(". . This is a dot file"));
            continue;
        }

        // 隠し属性を持つものは飛ばす
        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
        {
            DebugPrintLn(TEXT(". . This is a hidden file"));
            continue;
        }

        DebugPrintLn(TEXT(". Checked file attribute"));

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
                this->ScanDirectory(path);
            }
        }
        else
        {
            // DLLファイルだったら
            if ( lstrcmp(::PathFindExtension(path), TEXT(".dll")) == 0 )
            {
                // コンポーネントデータベースに記憶
                this->RegisterComponent(path);
            }
        }

        DebugPrintLn(TEXT(". Next file"));
    }
    while ( ::FindNextFile(hFindFile, &fd) == TRUE );

    ::FindClose( hFindFile );

    DebugPrintLn(NAME1 TEXT("::ScanDirectory() end"));
}

//---------------------------------------------------------------------------//

void __stdcall CompManager::Impl::RegisterComponent(LPCWSTR comp_path)
{
    DebugPrintLn(NAME1 TEXT("::RegisterComponent() begin"));
    DebugPrintLn(comp_path);

    ICompAdapter* adapter = nullptr;
    CLSID clsid = CLSID_NULL;

    // DLLファイルに含まれているクラスIDを全て取得
    for ( size_t index = 0; index < MAX_COMPONENT_COUNT ; ++index )
    {
        DebugPrintLn(TEXT(". Searching ClassIDs: index == %d"), index);

        // コンポーネント情報を管理するオブジェクトを生成
        adapter = new CompAdapter();
        auto hr = adapter->Load(comp_path, index);
        if ( FAILED(hr) )
        {
            DebugPrintLn(TEXT(". . No more component in this DLL"));
            adapter->Release();
            adapter = nullptr;
            break;
        }

        // コンポーネントのクラスIDを取得
        clsid = adapter->ClassID();

        // 同じクラスIDがすでに登録されていないか確認
        DebugPrintLn(TEXT(". Checking ClassID..."));
        {
            auto it = this->cmap.find(clsid);
            if ( it != this->cmap.end() )
            {
                DebugPrintLn(TEXT(". . Duplicate ClassID"));
                adapter->Release();
                adapter = nullptr;
                continue;
            }
        }
        DebugPrintLn(TEXT(". Checked ClassID: no duplicate"));

        // コンポーネント情報を本体のデータベースに記憶
        DebugPrintLn(TEXT(". Registering component..."));
        {
            this->cmap[clsid] = ComPtr<ICompAdapter>(adapter);
            // adapter はこのあと LoadAll() 内で配列にも参照を格納するので、
            // Release() メソッドは呼び出さない
        }
        DebugPrintLn(TEXT(". Registered component"));
    }
    DebugPrintLn(TEXT(". Searched ClassIDs"));

    DebugPrintLn(NAME1 TEXT("::RegisterComponent() end"));
}

//---------------------------------------------------------------------------//

CompManager::CompManager()
{
    DebugPrintLn(NAME1 TEXT("::Constructor() begin"));

    pimpl = new Impl;

    m_cRef = 0;

    this->AddRef();

    DebugPrintLn(NAME1 TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

CompManager::~CompManager()
{
    DebugPrintLn(NAME1 TEXT("::Destructor() begin"));

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME1 TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompManager::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME1 TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ICompManager) )
    {
        *ppvObject = static_cast<ICompManager*>(this);
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

ULONG __stdcall CompManager::AddRef()
{
    DebugPrintLn(NAME1 TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME1 TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall CompManager::Release()
{
    DebugPrintLn(NAME1 TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT(". Deleting..."));
        delete this;
        DebugPrintLn(TEXT(". Deleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME1 TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

size_t __stdcall CompManager::ComponentCount() const
{
    return pimpl->count;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall CompManager::DirectoryPath() const
{
    return pimpl->dir_path;
}

//---------------------------------------------------------------------------//

ICompAdapter* __stdcall CompManager::GetAt(size_t index) const
{
    DebugPrintLn(NAME1 TEXT("::GetAt() begin"));

    ICompAdapter* ca = nullptr;

    if ( index < pimpl->count )
    {
        ca = pimpl->comps[index];
        if ( ca )
        {
            ca->AddRef();
        }
    }

    DebugPrintLn(NAME1 TEXT("::GetAt() end"));

    return ca;
}

//---------------------------------------------------------------------------//

ICompAdapter* __stdcall CompManager::Find(REFCLSID rclsid) const
{
    DebugPrintLn(NAME1 TEXT("::Find() begin"));

    auto it = pimpl->cmap.find(rclsid);
    if ( it == pimpl->cmap.end() )
    {
        DebugPrintLn(TEXT(". Not found"));
        DebugPrintLn(NAME1 TEXT("::Find() end"));
        return nullptr;
    }

    auto adapter = it->second.GetInterface();
    if ( nullptr == adapter )
    {
        DebugPrintLn(TEXT(". GetInterface() failed"));
        DebugPrintLn(NAME1 TEXT("::Find() end"));
        return nullptr;
    }

    adapter->AddRef();

    DebugPrintLn(NAME1 TEXT("::Find() end"));

    return adapter;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompManager::LoadAll(LPCWSTR dir_path)
{
    DebugPrintLn(NAME1 TEXT("::LoadAll() begin"));

    // コンポーネントデータベースを一旦破棄
    if ( pimpl->count )
    {
        DebugPrintLn(TEXT(". Resetting component database..."));

        auto hr = this->FreeAll();
        if ( hr != S_OK )
        {
            DebugPrintLn(TEXT(". . Couldn't start loading"));
            return E_FAIL;
        }

        DebugPrintLn(TEXT(". Resetted component database"));
    }

    // フォルダのパスを内部変数にコピー
    ::StringCchPrintf(pimpl->dir_path, MAX_PATH, dir_path);

    // 自分自身をデータベースに登録
    WCHAR module_path[MAX_PATH];
    ::GetModuleFileName(nullptr, module_path, MAX_PATH);
    pimpl->RegisterComponent(module_path);

    // コンポーネントフォルダをスキャン
    pimpl->ScanDirectory(pimpl->dir_path);
    pimpl->count = pimpl->cmap.size();
    if ( pimpl->count < 1 )
    {
        DebugPrintLn(TEXT(". No component found"));
    }
    else
    {
        DebugPrintLn(TEXT(". No more component"));
    }

    // コンポーネントデータベースを構築
    pimpl->comps = new ICompAdapter*[1 + pimpl->count]; // 0でも空の配列を作る

    auto it  = pimpl->cmap.begin();
    auto itE = pimpl->cmap.end();
    for ( size_t i = 0; it != itE; ++i, ++it )
    {
        pimpl->comps[i] = it->second.GetInterface();
        // Impl::RegisterComponent() 内で参照カウントが増えたままなので、
        // ここで AddRef() メソッドは呼び出さない
    }
    pimpl->comps[pimpl->count] = nullptr; // 配列はNULL終端

    DebugPrintLn(NAME1 TEXT("::LoadAll() end"));

    return pimpl->count ? S_OK : S_FALSE;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall CompManager::FreeAll()
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
        DebugPrintLn(TEXT(". Freed nothing"));
        hr = E_FAIL;
    }
    else if ( freed == count )
    {
        DebugPrintLn(TEXT(". Freed all"));
        hr = S_OK;
    }
    else
    {
        DebugPrintLn(TEXT(". Did not free all"));
        hr = S_FALSE;
    }

    DebugPrintLn(NAME1 TEXT("::FreeAll() end"));

    return hr;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CompManager.cpp