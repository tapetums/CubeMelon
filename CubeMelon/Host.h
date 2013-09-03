// CubeMelon.Host.h

#pragma once

//---------------------------------------------------------------------------//

class IPluginManager;

class Host : public IPlugin, public IPluginHost
{
public:
    explicit Host(IUnknown* pUnkOuter);
    ~Host();

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

    IPluginManager* __stdcall PluginManager() const override;

protected:
    ULONG    m_cRef;
    IPlugin* m_owner;
    STATE    m_state;

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

// CubeMelon.Host.h