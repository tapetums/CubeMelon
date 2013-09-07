// CubeMelon.Host.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class Host : public IComponent
{
public:
    explicit Host(IUnknown* pUnkOuter);
    ~Host();

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

protected:
    ULONG       m_cRef;
    STATE       m_state;
    IComponent* m_owner;

private:
    struct Impl;
    Impl* pimpl;

private:
    Host(const Host&);
    Host(Host&&);
    Host& operator =(const Host&);
    Host& operator =(Host&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CubeMelon.Host.h