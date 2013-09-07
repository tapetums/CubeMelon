// Input.Wave.Wave.cpp

#include <windows.h>
#include <strsafe.h>
#include <mmreg.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "Wave.h"

//---------------------------------------------------------------------------//

#define NAME TEXT("Input.Wave")

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct Wave::Impl
{
    Impl();
    ~Impl();

    bool    IsSupportedFormat(LPCWSTR format);
    HRESULT Close();
    HRESULT Load();
    HRESULT Open(LPCWSTR path);

    UINT64               position;
    UINT64               size;
    HANDLE               hFile;
    HANDLE               hMap;
    BYTE*                pView;
    BYTE*                dataBegin;
    WAVEFORMATEXTENSIBLE wfex;
    WCHAR                path[MAX_PATH];
};

//---------------------------------------------------------------------------//

Wave::Impl::Impl()
{
    position  = 0;
    size      = 0;
    hFile     = INVALID_HANDLE_VALUE;
    hMap      = INVALID_HANDLE_VALUE;
    pView     = nullptr;
    dataBegin = nullptr;
    path[0]   = '\0';
    ::ZeroMemory(&wfex, sizeof(WAVEFORMATEXTENSIBLE));
}

//---------------------------------------------------------------------------//

Wave::Impl::~Impl()
{
    this->Close();
}

//---------------------------------------------------------------------------//

bool Wave::Impl::IsSupportedFormat(LPCWSTR format)
{
    if (format[0] == '.' )
    {
        ++format;
    }

    if ( lstrcmp(format, TEXT("wav"))  == 0 ||
         lstrcmp(format, TEXT("WAV"))  == 0 ||
         lstrcmp(format, TEXT("riff")) == 0 ||
         lstrcmp(format, TEXT("RIFF")) == 0 ||
         lstrcmp(format, TEXT("rf64")) == 0 ||
         lstrcmp(format, TEXT("RF64")) == 0 )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------//

HRESULT Wave::Impl::Close()
{
    DebugPrintLn(NAME TEXT("Impl::Close() begin"));

    if( pView )
    {
        ::UnmapViewOfFile(pView);
        pView = nullptr;
    }
    if ( hMap != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle(hMap);
        hMap = INVALID_HANDLE_VALUE;
    }
    if ( hFile != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    position  = 0;
    size      = 0;
    dataBegin = nullptr;
    path[0]   = '\0';
    ::ZeroMemory(&wfex, sizeof(WAVEFORMATEXTENSIBLE));

    DebugPrintLn(NAME TEXT("Impl::Close() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT Wave::Impl::Load()
{
    DebugPrintLn(NAME TEXT("Impl::Load() begin"));
    DebugPrintLn(path);

    try
    {
        CHAR  chunkId[4];
        DWORD chunkSize;
        CHAR  riffType[4];

        auto p = pView;
        ::CopyMemory(&chunkId,   p,     sizeof(chunkId));
        ::CopyMemory(&chunkSize, p + 4, sizeof(chunkSize));
        ::CopyMemory(&riffType,  p + 8, sizeof(riffType));
        if ( strncmp(chunkId, "RIFF", sizeof(chunkId)) != 0 &&
             strncmp(chunkId, "RF64", sizeof(chunkId)) != 0 )
        {
            throw TEXT("This file is neither RIFF nor RF64");
        }
        if ( strncmp(riffType, "WAVE", sizeof(chunkId)) != 0 )
        {
            throw TEXT("This file is not WAV");
        }
        p += 12;

        UINT64 fileSize = 12;
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            MEMORY_BASIC_INFORMATION mbi = { };
            fileSize = (UINT64)::VirtualQuery(pView, &mbi, sizeof(mbi));
        }
        else
        {
            LARGE_INTEGER li;
            ::GetFileSizeEx(hFile, &li);
            fileSize = li.QuadPart;
        }
        DebugPrintLn(TEXT("File size is %d"), fileSize);

        while ( p < pView + fileSize )
        {
            ::CopyMemory(&chunkId,   p,     sizeof(chunkId));
            ::CopyMemory(&chunkSize, p + 4, sizeof(chunkSize));

            DebugPrintLn(TEXT("%s"), chunkId);

            if ( strncmp(chunkId, "fmt ", sizeof(chunkId)) == 0 )
            {
                WORD tag = WAVE_FORMAT_UNKNOWN;
                ::CopyMemory(&tag, p + 8, sizeof(tag));

                if (tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_IEEE_FLOAT)
                {
                    ::CopyMemory(&wfex, p + 8, sizeof(PCMWAVEFORMAT));
                }
                else if (tag == WAVE_FORMAT_EXTENSIBLE)
                {
                    ::CopyMemory(&wfex, p + 8, sizeof(WAVEFORMATEXTENSIBLE));
                }
                else
                {
                    throw TEXT("UNKNOWN FORMAT");
                }
            }
            else if ( strncmp(chunkId, "ds64", sizeof(chunkId)) == 0 )
            {
                ::CopyMemory(&fileSize, p +  8, sizeof(fileSize));
                ::CopyMemory(&size,     p + 16, sizeof(size));
            }
            else if ( strncmp(chunkId, "data", sizeof(chunkId)) == 0 )
            {
                if ( chunkSize != (DWORD)-1 )
                {
                    size = chunkSize;
                }
                dataBegin = p + 8;
            }

            if ( chunkSize == (DWORD)-1 )
            {
                // 仕様書でも今のところchunkSizeが32bitオーバーするのは
                // RF64チャンクとdataチャンクだけだと言っている
                // （levlチャンクはファイルサイズが512GBを超えると4GB超になる）
                // EBU Tech 3306-2009 p.13
                p = p + sizeof(chunkId) + sizeof(chunkSize) + size;
            }
            else
            {
                p = p + sizeof(chunkId) + sizeof(chunkSize) + chunkSize;
            }
        }

        if ( dataBegin == nullptr )
        {
            throw TEXT("'data' chunk is not found");
        }
    }
    catch (LPCWSTR msg)
    {
        DebugPrintLn(msg);

        this->Close();

        return E_FAIL;
    }

    DebugPrintLn(NAME TEXT("Impl::Load() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT Wave::Impl::Open(LPCWSTR path)
{
    DebugPrintLn(NAME TEXT("Impl::Open() begin"));
    DebugPrintLn(path);

    // パスを内部領域にコピー
    ::StringCchCopy(this->path, MAX_PATH, path);

    // ファイル or 共有メモリを開く
    try
    {
        hFile = ::CreateFile
        (
            path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
            OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr
        );
        if ( hFile == INVALID_HANDLE_VALUE )
        {
            DebugPrintLn(TEXT("This is a memory mapped file"));

            hMap = ::OpenFileMapping
            (
                FILE_MAP_READ, FALSE, path
            );
        }
        else
        {
            DebugPrintLn(TEXT("This is a substantial file"));

            hMap = ::CreateFileMapping
            (
                hFile, nullptr, PAGE_READONLY,
                0, 0,
                nullptr
            );
        }
        if ( hMap <= 0 )
        {
            throw TEXT("File mapping failed");
        }

        pView = (BYTE*)::MapViewOfFile
        (
            hMap, FILE_MAP_READ, 0, 0, 0
        );
        if ( pView == nullptr )
        {
            throw TEXT("MapViewOfFile() failed");
        }
    }
    catch (LPCWSTR msg)
    {
        DebugPrintLn(msg);

        this->Close();

        return E_FAIL;
    }

    DebugPrintLn(NAME TEXT("Impl::Open() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

Wave::Wave(IUnknown* pUnkOuter)
{
    DebugPrintLn(NAME TEXT("::Constructor() begin"));

    pimpl = new Impl;

    m_cRef  = 0;
    m_state = STATE_IDLE;
    m_owner = nullptr;
    if ( pUnkOuter )
    {
        auto hr = pUnkOuter->QueryInterface
        (
            IID_IComponent, (void**)&m_owner
        );
        if ( FAILED(hr) )
        {
            m_owner = nullptr;
        }
    }

    this->AddRef();

    DebugPrintLn(NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

Wave::~Wave()
{
    DebugPrintLn(NAME TEXT("::Destructor() begin"));

    this->Stop();

    m_state = STATE_TERMINATING;
    if ( m_owner )
    {
        DebugPrintLn(TEXT("Releasing ") NAME TEXT("'s Owner..."));

        m_owner->Release();
        m_owner = nullptr;

        DebugPrintLn(TEXT("Released ") NAME TEXT("'s Owner"));
    }
    m_cRef = 0;

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(NAME TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(NAME TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IComponent) )
    {
        *ppvObject = static_cast<IComponent*>(this);
    }
    else if ( IsEqualIID(riid, IID_IIOComponent) )
    {
        *ppvObject = static_cast<IIOComponent*>(this);
    }
    else if ( IsEqualIID(riid, IID_IInputComponent) )
    {
        *ppvObject = static_cast<IInputComponent*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(NAME TEXT("::QueryInterface() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall Wave::AddRef()
{
    DebugPrintLn(NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall Wave::Release()
{
    DebugPrintLn(NAME TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(NAME TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

REFCLSID __stdcall Wave::ClassID() const
{
    return CLSID_Component;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall Wave::Owner() const
{
    return m_owner;
}

//---------------------------------------------------------------------------//

IPropertyStore* __stdcall Wave::Property() const
{
    return nullptr;
}

//---------------------------------------------------------------------------//

STATE __stdcall Wave::Status() const
{
    return m_state;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Attach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Detach(LPCWSTR msg, IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::GetInstance
(
    REFCLSID rclsid, REFIID riid, void** ppvObject
)
{
    if ( m_owner )
    {
        return m_owner->GetInstance(rclsid, riid, ppvObject);
    }
    else
    {
        return E_NOTIMPL;
    }
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Notify
(
    IComponent* sender, LPCWSTR msg, LPVOID data, size_t cb_data
)
{
    DebugPrintLn(NAME TEXT("::Notify() begin"));

    DebugPrintLn(NAME TEXT("::Notify() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Start(LPCVOID args)
{
    DebugPrintLn(NAME TEXT("::Start() begin"));

    HRESULT hr;

    if ( m_state == STATE_RUNNING )
    {
        DebugPrintLn(TEXT("Already started"));
        return S_FALSE;
    }

    /// ここに処理を書く

    m_state = STATE_RUNNING;

    DebugPrintLn(NAME TEXT("::Start() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Stop()
{
    DebugPrintLn(NAME TEXT("::Stop() begin"));

    if ( m_state == STATE_IDLE )
    {
        DebugPrintLn(TEXT("Already stopped"));
        return S_FALSE;
    }

    this->Close();

    m_state = STATE_IDLE;

    DebugPrintLn(NAME TEXT("::Stop() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Close(IComponent* listener)
{
    DebugPrintLn(NAME TEXT("::Close() begin"));

    if ( !(m_state & STATE_OPEN) )
    {
        return S_FALSE;
    }

    HRESULT hr;

    if ( listener )
    {
        //hr = pimpl->CloseAsync(pimpl, listener);
        hr = E_NOTIMPL;
    }
    else
    {
        hr = pimpl->Close();
        if ( SUCCEEDED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_OPEN);
        }
    }

    DebugPrintLn(NAME TEXT("::Close() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Open
(
    LPCWSTR path, LPCWSTR format_as, IComponent* listener
)
{
    DebugPrintLn(NAME TEXT("::Open() begin"));

    if ( !pimpl->IsSupportedFormat(format_as) )
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    if ( listener )
    {
        //hr = pimpl->OpenAsync(pimpl, path, listener);
        hr = E_NOTIMPL;
    }
    else
    {
        hr = pimpl->Open(path);
        if ( SUCCEEDED(hr) )
        {
            hr = pimpl->Load();
            if ( SUCCEEDED(hr) )
            {
                m_state = (STATE)(m_state | STATE_OPEN);
            }
        }
    }

    DebugPrintLn(NAME TEXT("::Open() end"));

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::QuerySupport(LPCWSTR path, LPCWSTR format_as)
{
    DebugPrintLn(NAME TEXT("::QuerySupport() begin"));

    if ( !pimpl->IsSupportedFormat(format_as) )
    {
        return E_FAIL;
    }

    DebugPrintLn(NAME TEXT("::QuerySupport() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Seek(INT64 offset, DWORD origin, UINT64* new_pos)
{
    DebugPrintLn(NAME TEXT("::Seek() begin"));

    switch ( origin )
    {
        case FILE_BEGIN:
        {
            pimpl->position = offset;
            break;
        }
        case FILE_CURRENT:
        {
            pimpl->position += offset;
            if ( pimpl->position > pimpl->size )
            {
                pimpl->position = pimpl->size;
            }
            break;
        }
        case FILE_END:
        {
            if ( offset > pimpl->size )
            {
                pimpl->position = 0;
            }
            else
            {
                pimpl->position = pimpl->size - offset;
            }
            break;
        }
        default:
        {
            return E_INVALIDARG;
        }
    }

    if ( new_pos )
    {
        *new_pos = pimpl->position;
    }

    DebugPrintLn(NAME TEXT("::Seek() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall Wave::Read
(
    LPVOID buffer, size_t buf_size, size_t* cb_read, IComponent* listener
)
{
    DebugPrintLn(NAME TEXT("::Read() begin"));

    DebugPrintLn(NAME TEXT("::Read() end"));

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.Wave.cpp