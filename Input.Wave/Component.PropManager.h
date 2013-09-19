// Input.Wave.Component.PropManager.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class CompProperty : public IProperty
{
public:
    CompProperty(LPCWSTR name, DATATYPE type, VARIANT* data, size_t size);
    ~CompProperty();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    VARIANT* __stdcall Data() const override;
    LPCWSTR  __stdcall Name() const override;
    size_t   __stdcall Size() const override;
    DATATYPE __stdcall Type() const override;

protected:
    ULONG    m_cRef;
    VARIANT* m_data;
    WCHAR*   m_name;
    size_t   m_size;
    DATATYPE m_type;

private:
    CompProperty(const CompProperty&);
    CompProperty(CompProperty&&);
    CompProperty& operator =(const CompProperty&);
    CompProperty& operator =(CompProperty&&);
};

//---------------------------------------------------------------------------//

class CompPropManager : public IPropManager
{
public:
    explicit CompPropManager(LPCWSTR path, WAVEFORMATEXTENSIBLE* wfex);
    ~CompPropManager();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    size_t __stdcall PropCount() const override;

    HRESULT __stdcall GetAt(size_t index, IProperty** prop) override;
    HRESULT __stdcall GetByName(LPCWSTR name, IProperty** prop) override;
    HRESULT __stdcall SetAt(size_t index, IProperty* prop) override;
    HRESULT __stdcall SetByName(LPCWSTR name, IProperty* prop) override;

protected:
    ULONG m_cRef;

    LPCWSTR m_path;
    WAVEFORMATEXTENSIBLE* m_wfex;

private:
    CompPropManager(const CompPropManager&);
    CompPropManager(CompPropManager&&);
    CompPropManager& operator =(const CompPropManager&);
    CompPropManager& operator =(CompPropManager&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.Component.PropManager.h