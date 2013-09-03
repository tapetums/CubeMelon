// IOPlugin.h

#pragma once

//---------------------------------------------------------------------------//

class IOPlugin : public IPlugin, public IIOPlugin
{
public:
    explicit IOPlugin(IUnknown* pUnkOuter);
    ~IOPlugin();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID __stdcall ClassID() const override;
    IPlugin* __stdcall Owner() const override;
    STATE    __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IPlugin* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IPlugin* listener) override;
    HRESULT __stdcall Notify(IPlugin* sender, LPCWSTR msg, LPVOID data, size_t cb_data) override;
    HRESULT __stdcall GetPluginInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Start(LPCVOID args = nullptr) override;
    HRESULT __stdcall Stop() override;

    HRESULT __stdcall Close(IPlugin* listener) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IPlugin* listener) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;
    HRESULT __stdcall Read(LPVOID buffer, size_t buf_size, size_t* cb_read) override;
    HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) override;
    HRESULT __stdcall Write(LPCVOID buffer, size_t buf_size, size_t* cb_written) override;

protected:
    ULONG    m_cRef;
    IPlugin* m_owner;
    STATE    m_state;

private:
    struct Impl;
    Impl* pimpl;

private:
    IOPlugin(const IOPlugin&);
    IOPlugin(IOPlugin&&);
    IOPlugin& operator =(const IOPlugin&);
    IOPlugin& operator =(IOPlugin&&);
};

//---------------------------------------------------------------------------//

// IOPlugin.h