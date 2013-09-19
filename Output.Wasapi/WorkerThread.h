// Output.Wasapi.WorkerThread.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

UINT __stdcall WriteWave(void* arglist);

//---------------------------------------------------------------------------//

struct QueueData
{
    LPVOID      buffer;
    size_t      buf_size;
    IComponent* listener;
};

//---------------------------------------------------------------------------//

struct InternalParams
{
    IComponent* sender;

    bool                  is_active;
    HANDLE                e_queued;
    HANDLE                e_writing;
    IMMDevice*            device;
    IAudioClient*         client;
    IAudioRenderClient*   renderer;
    CRITICAL_SECTION      cs;
    std::queue<QueueData> q;

    InternalParams();
    ~InternalParams();
};

//---------------------------------------------------------------------------//

class MsgObject : public IMsgObject
{
public:
    explicit MsgObject(IComponent* sender, IComponent* listener, LPVOID data, size_t data_size);
    ~MsgObject();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    LPCWSTR     __stdcall Name() const override;
    LPCWSTR     __stdcall Message() const override;
    IComponent* __stdcall Sender() const override;
    IComponent* __stdcall Listener() const override;
    size_t      __stdcall DataCount() const override;
    LPCWSTR     __stdcall DataName(size_t index = 0) const override;
    size_t      __stdcall DataSize(size_t index = 0) const override;
    DATATYPE    __stdcall DataType(size_t index = 0) const override;
    LPVOID      __stdcall Data(size_t index = 0) const override;

protected:
    ULONG m_cRef;

private:
    IComponent* m_sender;
    IComponent* m_listener;
    size_t      m_size;
    LPVOID      m_data;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Output.Wasapi.WorkerThread.h