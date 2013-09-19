﻿// Input.Wave.WorkerThread.cpp

#include <queue>

#include <windows.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"

#include "WorkerThread.h"

//---------------------------------------------------------------------------//

#define NAME        TEXT("Input.Wave::Internal")
#define MSGOBJ_NAME TEXT("Input.Wave.MsgObject")


//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

UINT __stdcall ReadWave(void* arglist)
{
    auto params = reinterpret_cast<InternalParams*>(arglist);
    if ( nullptr == params )
    {
        return (UINT)E_POINTER;
    }

    // よく使う値をローカル定数として記憶
    const auto sender   = params->sender;
    const auto cs       = &params->cs;
    const auto q        = &params->q;
    const auto e_queued = params->e_queued;

    QueueData data = { };
    while ( params->is_active )
    {
        // 読込キューからデータを取り出す
        ::EnterCriticalSection(cs);
        {
            if ( q->empty() )
            {
                data.buffer = nullptr;
                ::ResetEvent(e_queued);
            }
            else
            {
                data = q->front();
                q->pop();
            }
        }
        ::LeaveCriticalSection(cs);

        if ( nullptr == data.buffer )
        {
            // 読み込みキューが積まれるまで待つ
            ::WaitForSingleObject(e_queued, INFINITE);
        }
        else
        {
            // データをバッファにコピー
            // バッファサイズはキューに積む時点で正規化済み
            ::CopyMemory(data.buffer, data.src, data.buf_size);

            // 完了を通知
            auto msg_obj = new MsgObject
            (
                sender, data.listener, data.buffer, data.buf_size
            );
            sender->Notify(msg_obj);
            data.listener->Release();
        }
    }

    return (UINT)S_OK;
}

//---------------------------------------------------------------------------//

InternalParams::InternalParams()
{

    is_active = false;

    e_queued  = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
    ::ResetEvent(e_queued);

    ::InitializeCriticalSection(&cs);
}

//---------------------------------------------------------------------------//

InternalParams::~InternalParams()
{
    if ( e_queued )
    {
        ::CloseHandle(e_queued);
        e_queued = nullptr;
    }

    ::DeleteCriticalSection(&cs);
}

//---------------------------------------------------------------------------//

MsgObject::MsgObject
(
    IComponent* sender, IComponent* listener, LPVOID data, size_t data_size
)
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::Constructor() begin"));

    m_sender   = sender;
    m_listener = listener;
    m_data     = data;
    m_size     = data_size;

    if ( m_sender )
    {
        m_sender->AddRef();
    }
    if ( m_listener )
    {
        m_listener->AddRef();
    }

    DebugPrintLn(MSGOBJ_NAME TEXT("::Constructor() end"));
}

//---------------------------------------------------------------------------//

MsgObject::~MsgObject()
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::Destructor() begin"));

    if ( m_sender )
    {
        m_sender->Release();
        m_sender = nullptr;
    }
    if ( m_listener )
    {
        m_listener->Release();
        m_listener = nullptr;
    }

    DebugPrintLn(MSGOBJ_NAME TEXT("::Destructor() end"));
}

//---------------------------------------------------------------------------//

HRESULT __stdcall MsgObject::QueryInterface(REFIID riid, void** ppvObject)
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::QueryInterface() begin"));

    if ( nullptr == ppvObject )
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMsgObject) )
    {
        *ppvObject = static_cast<IMsgObject*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    DebugPrintLn(MSGOBJ_NAME TEXT("::QueryInterface() end"));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall MsgObject::AddRef()
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::AddRef() begin %d"), m_cRef);

    LockModule();

    LONG cRef = ::InterlockedIncrement(&m_cRef);

    DebugPrintLn(MSGOBJ_NAME TEXT("::AddRef() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

ULONG __stdcall MsgObject::Release()
{
    DebugPrintLn(MSGOBJ_NAME TEXT("::Release() begin %d"), m_cRef);

    LONG cRef = ::InterlockedDecrement(&m_cRef);
    if ( cRef == 0 )
    {
        DebugPrintLn(TEXT("Deleting..."));
        delete this;
        DebugPrintLn(TEXT("Deleted"));
    }

    UnlockModule();

    DebugPrintLn(MSGOBJ_NAME TEXT("::Release() end %d"), cRef);

    return static_cast<ULONG>(cRef);
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall MsgObject::Name() const
{
    return MSGOBJ_NAME;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall MsgObject::Message() const
{
    return MSG_IO_READ_DONE;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall MsgObject::Sender() const
{
    return m_sender;
}

//---------------------------------------------------------------------------//

IComponent* __stdcall MsgObject::Listener() const
{
    return m_listener;
}

//---------------------------------------------------------------------------//

size_t __stdcall MsgObject::DataCount() const
{
    return 1;
}

//---------------------------------------------------------------------------//

LPCWSTR __stdcall MsgObject::DataName(size_t index) const
{
    return TEXT("Wave Data");
}

//---------------------------------------------------------------------------//

size_t __stdcall MsgObject::DataSize(size_t index) const
{
    return m_size;
}

//---------------------------------------------------------------------------//

DATATYPE __stdcall MsgObject::DataType(size_t index) const
{
    return TYPE_PTR;
}

//---------------------------------------------------------------------------//

LPVOID __stdcall MsgObject::Data(size_t index) const
{
    return m_data;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.WorkerThread.cpp