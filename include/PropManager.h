// CubeMelon.PropManager.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class Property : public IProperty
{
public:
    Property(LPCWSTR name, DATATYPE type, VARIANT* data, size_t size);
    ~Property();

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
    Property(const Property&);
    Property(Property&&);
    Property& operator =(const Property&);
    Property& operator =(Property&&);
};

//---------------------------------------------------------------------------//

class PropManager : public IPropManager
{
public:
    PropManager();
    ~PropManager();

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

private:
    PropManager(const PropManager&);
    PropManager(PropManager&&);
    PropManager& operator =(const PropManager&);
    PropManager& operator =(PropManager&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CubeMelon.PropManager.h