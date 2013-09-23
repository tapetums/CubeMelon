// Output.Wasapi.Wasapi.cpp

#include <queue>

#include <windows.h>
#include <strsafe.h>
#include <mmreg.h>
#include <process.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <mmdeviceapi.h>
#include <audioclient.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\ComponentBase.h"

#include "Wasapi.h"
#include "WorkerThread.h"

//---------------------------------------------------------------------------//

#define THREAD_WAIT_TIME 1000

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component =
{ 0xff3fc2e6, 0x2ea5, 0x4c75, { 0x95, 0x92, 0x95, 0x78, 0x7a, 0x22, 0xa8, 0xc5 } };

extern const size_t      MDL_PROP_COUNT    = 5;
extern const wchar_t*    MDL_PROP_MGR_NAME = TEXT("Output.Wasapi.PropManager");
extern const wchar_t*    MDL_PROP_NAME     = TEXT("Output.Wasapi.Property");

extern const size_t      COMP_INDEX      = 0;
extern const wchar_t*    COMP_NAME       = TEXT("Output.Wasapi");

extern const wchar_t*    PropName        = TEXT("Output.Wasapi");
extern const wchar_t*    PropDescription = TEXT("Output component for Windows Audio Session API");
extern const wchar_t*    PropCopyright   = TEXT("(C) 2011-2013 tapetums");
extern const VersionInfo PropVersion     = { 1, 0, 0, 0 };

//---------------------------------------------------------------------------//

struct COMP_CLASS_NAME::Impl
{
    Impl();
    ~Impl();

    void ClearQueue();

    InternalParams* params;

    HANDLE writer_thread;

    WAVEFORMATEXTENSIBLE wfex;
};

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::Impl::Impl()
{
    params = new InternalParams;

    writer_thread = nullptr;

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

COMP_CLASS_NAME::COMP_CLASS_NAME(IUnknown* pUnkOuter) : OutputComponentBase(pUnkOuter)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_NAME);

    pimpl = new Impl;
    pimpl->params->sender = this;

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

HRESULT __stdcall COMP_CLASS_NAME::Notify(IMsgObject* msg_obj)
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
        if ( lstrcmp(msg, MSG_IO_WRITE_DONE) == 0 )
        {
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
            m_state = (STATE)(m_state ^ STATE_WRITING);
        }
        else if ( lstrcmp(msg, MSG_IO_WRITE_FAILED) == 0 )
        {
            m_state = (STATE)(m_state ^ STATE_WRITING);
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

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Start(LPVOID args, IComponent* listener)
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
        return S_FALSE;
        DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STARTING);

    pimpl->writer_thread = (HANDLE)_beginthreadex
    (
        nullptr, 0, WriteWave, pimpl->params, 0, nullptr
    );
    if ( nullptr == pimpl->writer_thread )
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

HRESULT __stdcall COMP_CLASS_NAME::Stop(IComponent* listener)
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

    if ( pimpl->writer_thread )
    {
        pimpl->params->is_active = false;
        ::SetEvent(pimpl->params->e_queued);

        auto result = ::WaitForSingleObject
        (
            pimpl->writer_thread, THREAD_WAIT_TIME
        );
        if ( result == WAIT_TIMEOUT )
        {
            DebugPrintLn(TEXT("Thread termination timed out"));
            return E_FAIL;
        }

        ::CloseHandle(pimpl->writer_thread);
        pimpl->writer_thread = nullptr;
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

    HRESULT hr;

    m_state = (STATE)(m_state | STATE_CLOSING);

    if ( listener )
    {
        hr = this->CloseAsync(listener);
        if ( FAILED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_CLOSING);
        }
    }
    else
    {
        hr = this->CloseSync();
        if ( SUCCEEDED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_OPEN);
        }
        m_state = (STATE)(m_state ^ STATE_CLOSING);
    }

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

    HRESULT hr;

    hr = this->QuerySupport(path, format_as);
    if ( FAILED(hr) )
    {
        DebugPrintLn(TEXT("Unsupported format: %s"), format_as);
        return E_INVALIDARG;
    }

    m_state = (STATE)(m_state | STATE_OPENING);

    ::StringCchCopy(m_path, MAX_PATH, path);
    DebugPrintLn(m_path);

    if ( listener )
    {
        hr = this->OpenAsync(listener);
        if ( FAILED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_OPENING);
        }
    }
    else
    {
        hr = this->OpenSync();
        if ( SUCCEEDED(hr) )
        {
            m_state = (STATE)(m_state | STATE_OPEN);
        }
        m_state = (STATE)(m_state ^ STATE_OPENING);
    }

    DebugPrintLn(TEXT("%s::Open() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::QuerySupport
(
    LPCWSTR path, LPCWSTR format_as
)
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

HRESULT __stdcall COMP_CLASS_NAME::Write
(
    LPVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::Write() begin"), COMP_NAME);

    if ( !(m_state & STATE_OPEN) )
    {
        DebugPrintLn(TEXT("The object is not yet open"));
        DebugPrintLn(TEXT("%s::Write() end"), COMP_NAME);
        return E_PENDING;
    }
    if ( m_state & STATE_WRITING )
    {
        DebugPrintLn(TEXT("Now writing"));
        DebugPrintLn(TEXT("%s::Write() end"), COMP_NAME);
        return S_FALSE;
    }

    HRESULT hr;

    m_state = (STATE)(m_state | STATE_WRITING);

    if ( listener )
    {
        hr = this->WriteAsync(buffer, buf_size, listener);
        if ( FAILED(hr) )
        {
            m_state = (STATE)(m_state ^ STATE_WRITING);
        }
    }
    else
    {
        hr = this->WriteSync(buffer, buf_size, cb_written);
        m_state = (STATE)(m_state ^ STATE_WRITING);
    }

    DebugPrintLn(TEXT("%s::Write() end"), COMP_NAME);

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
    DebugPrintLn(TEXT("%s::Close() begin"), COMP_NAME);

    pimpl->ClearQueue();

    DebugPrintLn(TEXT("%s::Close() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::CloseAsync(IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::OpenSync()
{
    DebugPrintLn(TEXT("%s::Open() begin"), COMP_NAME);

    DebugPrintLn(TEXT("%s::Open() end"), COMP_NAME);

    return E_NOTIMPL;

CLOSE_ENDPOINT:
    DebugPrintLn(TEXT("Failed to open endpoint"));
    this->CloseSync();

    return E_FAIL;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::OpenAsync(IComponent* listener)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::WriteSync
(
    LPVOID buffer, size_t buf_size, size_t* cb_written
)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

HRESULT COMP_CLASS_NAME::WriteAsync
(
    LPVOID buffer, size_t buf_size, IComponent* listener
)
{
    DebugPrintLn(TEXT("%s::WriteAsync() begin"), COMP_NAME);

    if ( nullptr == listener )
    {
        return E_POINTER;
    }

    listener->AddRef();

    QueueData data =
    {
        buffer, buf_size, listener
    };

    ::EnterCriticalSection(&pimpl->params->cs);
    {
        pimpl->params->q.push(data);
    }
    ::LeaveCriticalSection(&pimpl->params->cs);

    ::SetEvent(pimpl->params->e_queued);

    DebugPrintLn(TEXT("%s::WriteAsync() end"), COMP_NAME);

    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Output.Wasapi.Wasapi.cpp