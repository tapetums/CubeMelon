// UI.SimplePlayer.SimplePlayer.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class SimplePlayer : public IUIComponent
{
public:
    explicit SimplePlayer(IUnknown* pUnkOuter);
    ~SimplePlayer();

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

    size_t __stdcall WindowCount() const override;
    HWND   __stdcall Handle(size_t index) const override;
protected:
    ULONG       m_cRef;
    STATE       m_state;
    IComponent* m_owner;

private:
    struct Impl;
    Impl* pimpl;

private:
    SimplePlayer(const SimplePlayer&);
    SimplePlayer(SimplePlayer&&);
    SimplePlayer& operator =(const SimplePlayer&);
    SimplePlayer& operator =(SimplePlayer&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// UI.SimplePlayer.SimplePlayer.h