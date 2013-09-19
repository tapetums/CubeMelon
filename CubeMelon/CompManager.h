// CubeMelon.CompManager.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class CompAdapter : public ICompAdapter
{
public:
    CompAdapter();
    ~CompAdapter();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID      __stdcall ClassID() const override;
    LPCWSTR       __stdcall Copyright() const override;
    LPCWSTR       __stdcall Description() const override;
    LPCWSTR       __stdcall FilePath() const override;
    size_t        __stdcall Index() const override;
    IPropManager* __stdcall PropManager() const override;
    LPCWSTR       __stdcall Name() const override;
    VersionInfo*  __stdcall Version() const override;

    HRESULT __stdcall Load(LPCWSTR file_path, size_t index) override;
    HRESULT __stdcall Free() override;
    HRESULT __stdcall Configure(HWND hwndParent = nullptr) override;
    HRESULT __stdcall CreateInstance(IComponent* owner, REFIID riid, void** ppvObject) override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    CompAdapter(const CompAdapter&);
    CompAdapter(CompAdapter&&);
    CompAdapter& operator =(const CompAdapter&);
    CompAdapter& operator =(CompAdapter&&);
};

//---------------------------------------------------------------------------//

class CompManager : public ICompManager
{
public:
    CompManager();
    ~CompManager();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    size_t        __stdcall ComponentCount() const override;
    LPCWSTR       __stdcall DirectoryPath() const override;
    ICompAdapter* __stdcall GetAt(size_t index) const override;
    ICompAdapter* __stdcall Find(REFCLSID rclsid) const override;

    HRESULT __stdcall LoadAll(LPCWSTR dir_path) override;
    HRESULT __stdcall FreeAll() override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    CompManager(const CompManager&);
    CompManager(CompManager&&);
    CompManager& operator =(const CompManager&);
    CompManager& operator =(CompManager&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CubeMelon.CompManager.h