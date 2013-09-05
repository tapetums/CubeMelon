// ComponentProperty.h

#pragma once

//---------------------------------------------------------------------------//

class ComponentProperty : public IPropertyStore
{
public:
    ComponentProperty();
    ~ComponentProperty();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    HRESULT __stdcall GetCount(DWORD* cProps) override;
    HRESULT __stdcall GetAt(DWORD iProp, PROPERTYKEY* pkey) override;
    HRESULT __stdcall GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) override;
    HRESULT __stdcall SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) override;
    HRESULT __stdcall Commit() override;

private:
    struct Impl;
    Impl* pimpl;

private:
    ComponentProperty(const ComponentProperty&);
    ComponentProperty(ComponentProperty&&);
    ComponentProperty& operator =(const ComponentProperty&);
    ComponentProperty& operator =(ComponentProperty&&);
};

//---------------------------------------------------------------------------//

// ComponentProperty.h