// Output.Wasapi.Wasapi.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class Wasapi : public IOutputComponent
{
public:
    explicit Wasapi(IUnknown* pUnkOuter);
    ~Wasapi();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID        __stdcall ClassID() const override;
    IComponent*     __stdcall Owner() const override;
    IPropertyStore* __stdcall Property() const override;
    STATE           __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Notify(IComponent* sender, LPCWSTR msg, LPVOID data, size_t cb_data) override;
    HRESULT __stdcall Start(LPCVOID args = nullptr) override;
    HRESULT __stdcall Stop() override;

    HRESULT __stdcall Close(IComponent* listener = nullptr) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;
    HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) override;

    HRESULT __stdcall Write(LPCVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener = nullptr) override;

protected:
    ULONG       m_cRef;
    STATE       m_state;
    IComponent* m_owner;

private:
    struct Impl;
    Impl* pimpl;

private:
    Wasapi(const Wasapi&);
    Wasapi(Wasapi&&);
    Wasapi& operator =(const Wasapi&);
    Wasapi& operator =(Wasapi&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Output.Wasapi.Wasapi.h