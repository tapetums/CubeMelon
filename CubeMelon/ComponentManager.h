// ComponentManager.h

#pragma once

//---------------------------------------------------------------------------//

struct VersionInfo;
struct IPropertyStore;

class ComponentContainer : public IComponentContainer
{
public:
    explicit ComponentContainer(LPCWSTR file_path, size_t index);
    ~ComponentContainer();

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
    ComponentContainer(const ComponentContainer&);
    ComponentContainer(ComponentContainer&&);
    ComponentContainer& operator =(const ComponentContainer&);
    ComponentContainer& operator =(ComponentContainer&&);
};

//---------------------------------------------------------------------------//

class ComponentManager : public IComponentManager
{
public:
    explicit ComponentManager(LPCWSTR dir_path);
    ~ComponentManager();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    LPCWSTR              __stdcall DirectoryPath() const override;
    size_t               __stdcall ComponentCount() const override;
    IComponentContainer* __stdcall ComponentContainer(size_t index) const override;

    HRESULT __stdcall LoadAll() override;
    HRESULT __stdcall FreeAll() override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    ComponentManager(const ComponentManager&);
    ComponentManager(ComponentManager&&);
    ComponentManager& operator =(const ComponentManager&);
    ComponentManager& operator =(ComponentManager&&);
};

//---------------------------------------------------------------------------//

// ComponentManager.h