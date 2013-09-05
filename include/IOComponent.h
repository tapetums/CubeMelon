// IOComponent.h

#pragma once

//---------------------------------------------------------------------------//

class IOComponent : public IIOComponent
{
public:
    explicit IOComponent(IUnknown* pUnkOuter);
    ~IOComponent();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID    __stdcall ClassID() const override;
    IComponent* __stdcall Owner() const override;
    STATE       __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Notify(IComponent* sender, LPCWSTR msg, LPVOID data, size_t cb_data) override;
    HRESULT __stdcall GetComponentInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Start(LPCVOID args = nullptr) override;
    HRESULT __stdcall Stop() override;

    HRESULT __stdcall Close(IComponent* listener) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;
    HRESULT __stdcall Read(LPVOID buffer, size_t buf_size, size_t* cb_read) override;
    HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) override;
    HRESULT __stdcall Write(LPCVOID buffer, size_t buf_size, size_t* cb_written) override;

protected:
    ULONG       m_cRef;
    IComponent* m_owner;
    STATE       m_state;

private:
    struct Impl;
    Impl* pimpl;

private:
    IOComponent(const IOComponent&);
    IOComponent(IOComponent&&);
    IOComponent& operator =(const IOComponent&);
    IOComponent& operator =(IOComponent&&);
};

//---------------------------------------------------------------------------//

// IOComponent.h