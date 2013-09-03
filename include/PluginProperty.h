// PluginProperty.h

#pragma once

//---------------------------------------------------------------------------//

class PluginProperty : public IPropertyStore
{
public:
    PluginProperty();
    ~PluginProperty();

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
    PluginProperty(const PluginProperty&);
    PluginProperty(PluginProperty&&);
    PluginProperty& operator =(const PluginProperty&);
    PluginProperty& operator =(PluginProperty&&);
};

// PluginProperty.h