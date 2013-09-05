// Component.h

#pragma once

//---------------------------------------------------------------------------//

class Component : public IComponent
{
public:
    explicit Component(IUnknown* pUnkOuter);
    ~Component();

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

protected:
    ULONG       m_cRef;
    IComponent* m_owner;
    STATE       m_state;

private:
    struct Impl;
    Impl* pimpl;

private:
    Component(const Component&);
    Component(Component&&);
    Component& operator =(const Component&);
    Component& operator =(Component&&);
};

//---------------------------------------------------------------------------//

// Component.h