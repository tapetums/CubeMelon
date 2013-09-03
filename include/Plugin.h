// Plugin.h

#pragma once

//---------------------------------------------------------------------------//

class Plugin : public IPlugin
{
public:
    explicit Plugin(IUnknown* pUnkOuter);
    ~Plugin();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    ::PluginManager* __stdcall PluginManager() const override;

    REFCLSID __stdcall ClassID() const override;
    IPlugin* __stdcall Owner() const override;
    STATE    __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IPlugin* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IPlugin* listener) override;
    HRESULT __stdcall Notify(IPlugin* sender, LPCWSTR msg, LPVOID data, size_t cb_data) override;
    HRESULT __stdcall GetPluginInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Start(LPCVOID args = nullptr) override;
    HRESULT __stdcall Stop() override;

protected:
    ULONG    m_cRef;
    IPlugin* m_owner;
    STATE    m_state;

private:
    struct Impl;
    Impl* pimpl;

private:
    Plugin(const Plugin&);
    Plugin(Plugin&&);
    Plugin& operator =(const Plugin&);
    Plugin& operator =(Plugin&&);
};

// Plugin.h