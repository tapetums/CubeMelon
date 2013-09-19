// Input.Wave.Component.PropManager.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class WaveProperty : public IPropertyStore
{
public:
    WaveProperty(WAVEFORMATEXTENSIBLE* wfex);
    ~WaveProperty();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    HRESULT __stdcall GetCount(DWORD* cProps) override;
    HRESULT __stdcall GetAt(DWORD iProp, PROPERTYKEY* pkey) override;
    HRESULT __stdcall GetValue(REFPROPERTYKEY key, PROPVARIANT* pv) override;
    HRESULT __stdcall SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar) override;
    HRESULT __stdcall Commit() override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    WaveProperty(const WaveProperty&);
    WaveProperty(WaveProperty&&);
    WaveProperty& operator =(const WaveProperty&);
    WaveProperty& operator =(WaveProperty&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.Component.PropManager.h