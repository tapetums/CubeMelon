// Input.Wave.Wave.cpp

#include <queue>

#include <windows.h>
#include <strsafe.h>
#include <mmreg.h>
#include <process.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\ComponentBase.h"

#include "Component.PropManager.h"
#include "Wave.h"
#include "WorkerThread.h"

//---------------------------------------------------------------------------//

#define THREAD_WAIT_TIME 1000

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component =
{ 0xae2c31a0, 0x7f16, 0x4e0d, { 0xb5, 0x2f, 0x22, 0x8e, 0xa0, 0x85, 0x15, 0x8f } };

extern const size_t      MDL_PROP_COUNT    = 5;
extern const wchar_t*    MDL_PROP_MGR_NAME = TEXT("Input.Wave.PropManager");
extern const wchar_t*    MDL_PROP_NAME     = TEXT("Input.Wave.Property");

extern const size_t      COMP_INDEX      = 0;
extern const wchar_t*    COMP_NAME       = TEXT("Input.Wave");

extern const wchar_t*    PropName        = TEXT("Input.Wave");
extern const wchar_t*    PropDescription = TEXT("Input component for WAVE and RF64");
extern const wchar_t*    PropCopyright   = TEXT("(C) 2012-2013 tapetums");
extern const VersionInfo PropVersion     = { 1, 0, 0, 0 };

//---------------------------------------------------------------------------//

struct COMP_CLASS_NAME::Impl
{
    Impl();
    ~Impl();

    void ClearQueue();

    InternalParams* params;

    HANDLE hFile;
    HANDLE hMap;
    BYTE*  pView;
    BYTE*  data_origin;
    HANDLE reader_thread;

    WAVEFORMATEXTENSIBLE wfex;
};

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::Impl::Impl()
{
    params = new InternalParams;

    hFile         = INVALID_HANDLE_VALUE;
    hMap          = INVALID_HANDLE_VALUE;
    pView         = nullptr;
    data_origin   = nullptr;
    reader_thread = nullptr;

    ::ZeroMemory(&wfex, sizeof(WAVEFORMATEXTENSIBLE));
}

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::Impl::~Impl()
{
    delete params;
    params = nullptr;
}

//---------------------------------------------------------------------------//

void COMP_CLASS_NAME::Impl::ClearQueue()
{
    QueueData data = { };
    const auto q = &params->q;

    ::EnterCriticalSection(&params->cs);
    {
        while ( !q->empty() )
        {
            data = q->back();
            q->pop();

            data.listener->Release();
            data.listener = nullptr;
        }
    }
    ::LeaveCriticalSection(&params->cs);
}

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::COMP_CLASS_NAME(IUnknown* pUnkOuter) : InputComponentBase(pUnkOuter)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_NAME);

    pimpl = new Impl;
    pimpl->params->sender = this;

    m_prop_mgr = new CompPropManager(m_path, &pimpl->wfex);

    DebugPrintLn(TEXT("%s::Constructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::~COMP_CLASS_NAME()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), COMP_NAME);

    HRESULT hr;

    hr = E_FAIL;
    while ( FAILED(hr) )
    {
        hr = this->Stop();
        ::MsgWaitForMultipleObjects(0, nullptr, FALSE, THREAD_WAIT_TIME, QS_ALLINPUT);
    }

    hr = E_FAIL;
    while ( FAILED(hr) )
    {
        hr = this->Close();
        ::MsgWaitForMultipleObjects(0, nullptr, FALSE, THREAD_WAIT_TIME, QS_ALLINPUT);
    }

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(TEXT("%s::Destructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Notify
(
    IMsgObject* msg_obj
)
{
    DebugPrintLn(TEXT("%s::Notify() begin"), COMP_NAME);

    if ( nullptr == msg_obj )
    {
        DebugPrintLn(TEXT("Message object is void"));
        return E_POINTER;
    }

    HRESULT hr;

    auto const sender = msg_obj->Sender();
    auto const name   = msg_obj->Name();
    auto const msg    = msg_obj->Message();

    DebugPrintLn(TEXT("%s: %s"), name, msg);

    if ( sender == this )
    {
        if ( lstrcmp(msg, MSG_IO_READ_DONE) == 0 )
        {
            m_position += msg_obj->DataSize();
            auto const listener = msg_obj->Listener();
            if ( listener )
            {
                msg_obj->AddRef();
                hr = listener->Notify(msg_obj);
            }
            else
            {
                hr = S_OK;
            }
            m_state = (STATE)(m_state ^ STATE_READING);
        }
        else if ( lstrcmp(msg, MSG_IO_READ_FAILED) == 0 )
        {
            m_state = (STATE)(m_state ^ STATE_READING);
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = S_FALSE;
    }

    msg_obj->Release();
    msg_obj = nullptr;

    DebugPrintLn(TEXT("%s::Notify() end"), COMP_NAME);

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Start
(
    LPVOID args, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::Start() begin"), COMP_NAME);

    if ( m_state & STATE_STARTING )
    {
        DebugPrintLn(TEXT("Now starting"));
        DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);
        return S_FALSE;
    }
    if ( m_state & STATE_ACTIVE )
    {
        DebugPrintLn(TEXT("Already started"));
        DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STARTING);

    pimpl->reader_thread = (HANDLE)_beginthreadex
    (
        nullptr, 0, ReadWave, pimpl->params, 0, nullptr
    );
    if ( nullptr == pimpl->reader_thread )
    {
        DebugPrintLn(TEXT("Could not create reader thread"));
        return E_FAIL;
    }

    m_state = (STATE)(m_state | STATE_ACTIVE);
    m_state = (STATE)(m_state ^ STATE_STARTING);

    DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Stop
(
    IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::Stop() begin"), COMP_NAME);

    if ( m_state & STATE_STOPPING )
    {
        DebugPrintLn(TEXT("Now stopping"));
        DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);
        return S_FALSE;
    }
    if ( !(m_state & STATE_ACTIVE) )
    {
        DebugPrintLn(TEXT("Already stopped"));
        DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STOPPING);

    if ( pimpl->reader_thread )
    {
        pimpl->params->is_active = false;
        ::SetEvent(pimpl->params->e_queued);

        auto result = ::WaitForSingleObject
        (
            pimpl->reader_thread, THREAD_WAIT_TIME
        );
        if ( result == WAIT_TIMEOUT )
        {
            DebugPrintLn(TEXT("Thread termination timed out"));
            return E_FAIL;
        }

        ::CloseHandle(pimpl->reader_thread);
        pimpl->reader_thread = nullptr;
    }

    m_state = (STATE)(m_state ^ STATE_ACTIVE);
    m_state = (STATE)(m_state ^ STATE_STOPPING);

    DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Close
(
    IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::Close() begin"), COMP_NAME);

    if ( m_state & STATE_ACTIVE )
    {
        DebugPrintLn(TEXT("Stop component before closing"));
        DebugPrintLn(TEXT("%s::Close() end"), COMP_NAME);
        return E_COMP_BUSY;
    }
    if ( m_state & STATE_CLOSING )
    {
        DebugPrintLn(TEXT("Now closing"));
        DebugPrintLn(TEXT("%s::Close() end"), COMP_NAME);
        return S_FALSE;
    }
    if ( !(m_state & STATE_OPEN) )
    {
        DebugPrintLn(TEXT("Already closed"));
        DebugPrintLn(TEXT("%s::Close() end"), COMP_NAME);
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    HRESULT hr;

    m_state = (STATE)(m_state | STATE_CLOSING);
    {
        hr = this->CloseSync();
        if ( SUCCEEDED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_OPEN);
        }
    }
    m_state = (STATE)(m_state ^ STATE_CLOSING);

    DebugPrintLn(TEXT("%s::Close() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Open
(
    LPCWSTR path, LPCWSTR format_as, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::Open() begin"), COMP_NAME);

    if ( m_state & STATE_ACTIVE )
    {
        DebugPrintLn(TEXT("Stop component before opening"));
        DebugPrintLn(TEXT("%s::Open() end"), COMP_NAME);
        return E_COMP_BUSY;
    }
    if ( m_state & STATE_OPENING )
    {
        DebugPrintLn(TEXT("Now opening"));
        DebugPrintLn(TEXT("%s::Open() end"), COMP_NAME);
        return S_FALSE;
    }
    if ( m_state & STATE_OPEN )
    {
        DebugPrintLn(TEXT("Already open"));
        DebugPrintLn(TEXT("%s::Open() end"), COMP_NAME);
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    HRESULT hr;

    hr = this->QuerySupport(path, format_as);
    if ( FAILED(hr) )
    {
        DebugPrintLn(TEXT("Unsupported format: %s"), format_as);
        return E_INVALIDARG;
    }

    m_state = (STATE)(m_state | STATE_OPENING);
    {
        ::StringCchCopy(m_path, MAX_PATH, path);
        DebugPrintLn(m_path);

        hr = this->OpenSync();
        if ( SUCCEEDED(hr) )
        {
            hr = this->LoadSync();
            if ( SUCCEEDED(hr) )
            {
                m_state = (STATE)(m_state | STATE_OPEN);
            }
        }
    }
    m_state = (STATE)(m_state ^ STATE_OPENING);

    DebugPrintLn(TEXT("%s::Open() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::QuerySupport(LPCWSTR path, LPCWSTR format_as)
{
    DebugPrintLn(TEXT("%s::QuerySupport() begin"), COMP_NAME);

    DebugPrintLn(TEXT("%s as %s"), path, format_as);

    HRESULT hr;

    if ( !this->IsSupportedExtension(path) )
    {
        hr = E_FAIL;
    }
    else if ( !this->IsSupportedFormat(format_as) )
    {
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;
    }

    DebugPrintLn(TEXT("%s::QuerySupport() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Seek(INT64 offset, DWORD origin, UINT64* new_pos)
{
    DebugPrintLn(TEXT("%s::Seek() begin"), COMP_NAME);

    if ( !(m_state & STATE_OPEN) )
    {
        DebugPrintLn(TEXT("The object is not yet open"));
        return E_PENDING;
    }
    if ( m_state & STATE_SEEKING )
    {
        DebugPrintLn(TEXT("Now seeking"));
        return E_COMP_BUSY;
    }

    DebugPrintLn(TEXT("cur position: %020llu"), m_position);

    if ( offset != 0 )
    {
        m_state = (STATE)(m_state | STATE_SEEKING);

        if ( origin == FILE_BEGIN )
        {
            if ( offset < 0 )
            {
                m_position = 0;
            }
            else
            {
                m_position = offset;
            }
        }
        else if ( origin == FILE_CURRENT )
        {
            m_position += offset;

            // position は uint64 なので、負数になった場合
            // 最上位ビットが立つため position > 2^63 になる
            if ( m_position > m_size )
            {
                if ( offset < 0 )
                {
                    m_position = 0;
                }
                else
                {
                    m_position = m_size;
                }
            }
        }
        else
        {
            if ( offset > 0 )
            {
                m_position = m_size;
            }
            else
            {
                m_position = m_size - offset;
            }
        }

        m_state = (STATE)(m_state ^ STATE_SEEKING);
    }
    if ( new_pos )
    {
        *new_pos = m_position;
    }

    DebugPrintLn(TEXT("new position: %020llu"), m_position);

    DebugPrintLn(TEXT("%s::Seek() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Read
(
    LPVOID buffer, size_t buf_size, size_t* cb_read, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::Read() begin"), COMP_NAME);

    if ( !(m_state & STATE_OPEN) )
    {
        DebugPrintLn(TEXT("The object is not yet open"));
        return E_FAIL;
    }
    if ( m_state & STATE_READING )
    {
        DebugPrintLn(TEXT("Now reading"));
        return S_FALSE;
    }

    if ( buf_size > m_size - m_position )
    {
        buf_size = m_size - m_position;
    }

    HRESULT hr;

    m_state = (STATE)(m_state | STATE_READING);

    if ( listener )
    {
        hr = this->ReadAsync(buffer, buf_size, listener);
        if ( FAILED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_READING);
        }
    }
    else
    {
        hr = this->ReadSync(buffer, buf_size, cb_read);
        m_state = (STATE)(m_state ^ STATE_READING);
    }

    DebugPrintLn(TEXT("%s::Read() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

bool COMP_CLASS_NAME::IsSupportedExtension(LPCWSTR path) const
{
    if ( nullptr == path )
    {
        return false;
    }

    DebugPrintLn(TEXT("%s::IsSupportedExtension(): begin"), COMP_NAME);

    auto ext = ::PathFindExtension(path);
    if ( ext[0] == '.' )
    {
        ++ext;
    }

    DebugPrintLn(TEXT("%s"), ext);

    bool ret;

    if ( lstrcmp(ext, TEXT("wav"))  == 0 ||
         lstrcmp(ext, TEXT("WAV"))  == 0 ||
         lstrcmp(ext, TEXT("riff")) == 0 ||
         lstrcmp(ext, TEXT("RIFF")) == 0 ||
         lstrcmp(ext, TEXT("rf64")) == 0 ||
         lstrcmp(ext, TEXT("RF64")) == 0 )
    {
        DebugPrintLn(TEXT("OK! Supported extension"));
        ret = true;
    }
    else
    {
        DebugPrintLn(TEXT("Unsupported extension"));
        ret = false;
    }

    DebugPrintLn(TEXT("%s::IsSupportedExtension(): end"), COMP_NAME);

    return ret;
}

//---------------------------------------------------------------------------//

bool COMP_CLASS_NAME::IsSupportedFormat(LPCWSTR format) const
{
    if ( nullptr == format )
    {
        return false;
    }

    DebugPrintLn(TEXT("%s::IsSupportedFormat(): begin"), COMP_NAME);

    if ( format[0] == '.' )
    {
        ++format;
    }

    DebugPrintLn(TEXT("%s"), format);

    bool ret;

    if ( lstrcmp(format, TEXT("wav"))  == 0 ||
         lstrcmp(format, TEXT("audio/wav"))  == 0 ||
         lstrcmp(format, TEXT("audio/wave"))  == 0 ||
         lstrcmp(format, TEXT("audio/x-wav"))  == 0 ||
         lstrcmp(format, TEXT("audio/vnd.wave"))  == 0 )
    {
        DebugPrintLn(TEXT("OK! Supported format"));
        ret = true;
    }
    else
    {
        DebugPrintLn(TEXT("Unsupported format"));
        ret = false;
    }

    DebugPrintLn(TEXT("%s::IsSupportedFormat(): end"), COMP_NAME);

    return ret;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::CloseSync()
{
    DebugPrintLn(TEXT("%s::CloseSync() begin"), COMP_NAME);

    pimpl->ClearQueue();

    if( pimpl->pView )
    {
        ::UnmapViewOfFile(pimpl->pView);
        pimpl->pView = nullptr;
    }
    if ( pimpl->hMap != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle(pimpl->hMap);
        pimpl->hMap = INVALID_HANDLE_VALUE;
    }
    if ( pimpl->hFile != INVALID_HANDLE_VALUE )
    {
        ::CloseHandle(pimpl->hFile);
        pimpl->hFile = INVALID_HANDLE_VALUE;
    }

    m_position = 0;
    m_size     = 0;
    m_path[0]  = '\0';

    pimpl->data_origin = nullptr;
    ::ZeroMemory(&pimpl->wfex, sizeof(WAVEFORMATEXTENSIBLE));

    DebugPrintLn(TEXT("%s::CloseSync() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::OpenSync()
{
    DebugPrintLn(TEXT("%s::OpenSync() begin"), COMP_NAME);

    WCHAR* err_msg = nullptr;

    // ファイル or 共有メモリを開く
    pimpl->hFile = ::CreateFile
    (
        m_path, GENERIC_READ, FILE_SHARE_READ, nullptr,
        OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr
    );
    if ( pimpl->hFile == INVALID_HANDLE_VALUE )
    {
        DebugPrintLn(TEXT("This is a memory mapped file"));

        pimpl->hMap = ::OpenFileMapping
        (
            FILE_MAP_READ, FALSE, m_path
        );
    }
    else
    {
        DebugPrintLn(TEXT("This is a substantial file"));

        pimpl->hMap = ::CreateFileMapping
        (
            pimpl->hFile, nullptr, PAGE_READONLY,
            0, 0, nullptr
        );
    }
    if ( pimpl->hMap <= 0 )
    {
        err_msg = TEXT("File mapping failed");
        goto OPENSYNC_CLOSE_FILE;
    }

    pimpl->pView = (BYTE*)::MapViewOfFile
    (
        pimpl->hMap, FILE_MAP_READ, 0, 0, 0
    );
    if ( pimpl->pView == nullptr )
    {
        err_msg = TEXT("MapViewOfFile() failed");
        goto OPENSYNC_CLOSE_FILE;
    }

    DebugPrintLn(TEXT("%s::OpenSync() end"), COMP_NAME);

    return S_OK;

OPENSYNC_CLOSE_FILE:
    DebugPrintLn(err_msg);
    DebugPrintLn(TEXT("This program is under %d-bit system"), 8 * sizeof(size_t));

    this->CloseSync();

    return E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::LoadSync()
{
    DebugPrintLn(TEXT("%s::Load() begin"), COMP_NAME);

    WCHAR* err_msg = nullptr;

    CHAR  chunkId[4];
    DWORD chunkSize;
    CHAR  riffType[4];

    auto p = pimpl->pView;
    ::CopyMemory(&chunkId,   p,     sizeof(chunkId));
    ::CopyMemory(&chunkSize, p + 4, sizeof(chunkSize));
    ::CopyMemory(&riffType,  p + 8, sizeof(riffType));
    if ( strncmp(chunkId, "RIFF", sizeof(chunkId)) != 0 &&
         strncmp(chunkId, "RF64", sizeof(chunkId)) != 0 )
    {
        err_msg = TEXT("This file is neither RIFF nor RF64");
        goto LOADSYNC_CLOSE_FILE;
    }
    if ( strncmp(riffType, "WAVE", sizeof(chunkId)) != 0 )
    {
        err_msg = TEXT("This file is not WAV");
        goto LOADSYNC_CLOSE_FILE;
    }
    p += 12;

    UINT64 fileSize = 12;
    if ( pimpl->hFile == INVALID_HANDLE_VALUE )
    {
        MEMORY_BASIC_INFORMATION mbi = { };
        fileSize = (UINT64)::VirtualQuery(pimpl->pView, &mbi, sizeof(mbi));
    }
    else
    {
        LARGE_INTEGER li;
        ::GetFileSizeEx(pimpl->hFile, &li);
        fileSize = li.QuadPart;
    }
    DebugPrintLn(TEXT("File size is %lld"), fileSize);

    while ( p < pimpl->pView + fileSize )
    {
        ::CopyMemory(&chunkId,   p,     sizeof(chunkId));
        ::CopyMemory(&chunkSize, p + 4, sizeof(chunkSize));

        #if defined(_DEBUG) || defined(DEBUG) // Debugのとき
        {
            WCHAR buf[5];
            ::ZeroMemory(buf, sizeof(WCHAR) * 5);
            for ( size_t i = 0; i < 4; ++i )
            {
                buf[i] = chunkId[i];
            }
            DebugPrintLn(TEXT("Chunk Id: %s"), buf);
        }
        #endif

        if ( strncmp(chunkId, "fmt ", sizeof(chunkId)) == 0 )
        {
            WORD tag = WAVE_FORMAT_UNKNOWN;
            ::CopyMemory(&tag, p + 8, sizeof(tag));
            ::ZeroMemory(&pimpl->wfex, sizeof(WAVEFORMATEXTENSIBLE));

            if ( tag == WAVE_FORMAT_PCM || tag == WAVE_FORMAT_IEEE_FLOAT )
            {
                ::CopyMemory(&pimpl->wfex, p + 8, sizeof(PCMWAVEFORMAT));
            }
            else if ( tag == WAVE_FORMAT_EXTENSIBLE )
            {
                ::CopyMemory(&pimpl->wfex, p + 8, sizeof(WAVEFORMATEXTENSIBLE));
            }
            else
            {
                err_msg = TEXT("UNKNOWN FORMAT");
                goto LOADSYNC_CLOSE_FILE;
            }
        }
        else if ( strncmp(chunkId, "ds64", sizeof(chunkId)) == 0 )
        {
            ::CopyMemory(&fileSize, p +  8, sizeof(fileSize));
            ::CopyMemory(&m_size,   p + 16, sizeof(m_size));
            DebugPrintLn(TEXT("Data size is %lld"), m_size);
        }
        else if ( strncmp(chunkId, "data", sizeof(chunkId)) == 0 )
        {
            if ( chunkSize != (DWORD)-1 )
            {
                m_size = chunkSize;
                DebugPrintLn(TEXT("Data size is %lld"), m_size);
            }
            pimpl->data_origin = p + 8;
        }

        if ( chunkSize == (DWORD)-1 )
        {
            // 仕様書でも今のところchunkSizeが32bitオーバーするのは
            // RF64チャンクとdataチャンクだけだと言っている
            // （levlチャンクはファイルサイズが512GBを超えると4GB超になる）
            // EBU Tech 3306-2009 p.13
            p = p + sizeof(chunkId) + sizeof(chunkSize) + m_size;
        }
        else
        {
            p = p + sizeof(chunkId) + sizeof(chunkSize) + chunkSize;
        }
    }

    if ( pimpl->data_origin == nullptr )
    {
        err_msg = TEXT("'data' chunk is not found");
        goto LOADSYNC_CLOSE_FILE;
    }

    DebugPrintLn(TEXT("%s::Load() end"), COMP_NAME);

    return S_OK;

LOADSYNC_CLOSE_FILE:
    DebugPrintLn(err_msg);

    this->CloseSync();

    return E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::ReadSync
(
    LPVOID buffer, size_t buf_size, size_t* cb_read
)
{
    DebugPrintLn(TEXT("%s::ReadSync() begin"), COMP_NAME);

    if ( nullptr == buffer )
    {
        return E_POINTER;
    }

    ::CopyMemory
    (
        buffer, pimpl->data_origin + m_position, buf_size
    );

    m_state = (STATE)(m_state | STATE_SEEKING);
    {
        m_position += buf_size;
    }
    m_state = (STATE)(m_state ^ STATE_SEEKING);

    if ( cb_read )
    {
        *cb_read = buf_size;
    }

    DebugPrintLn(TEXT("%s::ReadSync() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::ReadAsync
(
    LPVOID buffer, size_t buf_size, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::ReadAsync() begin"), COMP_NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    listener->AddRef();

    QueueData data =
    {
        buffer, pimpl->data_origin + m_position, buf_size, listener
    };

    ::EnterCriticalSection(&pimpl->params->cs);
    {
        pimpl->params->q.push(data);
    }
    ::LeaveCriticalSection(&pimpl->params->cs);

    ::SetEvent(pimpl->params->e_queued);

    DebugPrintLn(TEXT("%s::ReadAsync() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.Wave.cpp