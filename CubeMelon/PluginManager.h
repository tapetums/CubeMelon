// PluginManager.h

#pragma once

//---------------------------------------------------------------------------//

struct IPropertyStore;

class PluginContainer : public IPluginContainer
{
public:
    explicit PluginContainer(LPCWSTR file_path, size_t index);
    ~PluginContainer();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID     __stdcall ClassID() const override;
    LPCWSTR      __stdcall Copyright() const override;
    LPCWSTR      __stdcall Description() const override;
    LPCWSTR      __stdcall FilePath() const override;
    size_t       __stdcall Index() const override;
    LPCWSTR      __stdcall Name() const override;
    VersionInfo* __stdcall Version() const override;

    HRESULT __stdcall Load() override;
    HRESULT __stdcall Free() override;
    HRESULT __stdcall GetProperty(IPropertyStore** ps) override;
    HRESULT __stdcall GetClassObject(REFIID riid, void** ppvObject) override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    PluginContainer(const PluginContainer&);
    PluginContainer(PluginContainer&&);
    PluginContainer& operator =(const PluginContainer&);
    PluginContainer& operator =(PluginContainer&&);
};

//---------------------------------------------------------------------------//

class PluginManager : public IPluginManager
{
public:
    explicit PluginManager(LPCWSTR dir_path);
    ~PluginManager();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    LPCWSTR           __stdcall DirectoryPath() const override;
    size_t            __stdcall PluginCount() const override;
    IPluginContainer* __stdcall PluginContainer(size_t index) const override;

    HRESULT __stdcall LoadAll() override;
    HRESULT __stdcall FreeAll() override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    PluginManager(const PluginManager&);
    PluginManager(PluginManager&&);
    PluginManager& operator =(const PluginManager&);
    PluginManager& operator =(PluginManager&&);
};

//---------------------------------------------------------------------------//

// PluginManager.h