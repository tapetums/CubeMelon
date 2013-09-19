// Input.Wave.WasapiProperty.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class WasapiProperty : public IPropertyArray
{
public:
    WasapiProperty(WAVEFORMATEXTENSIBLE* wfex);
    ~WasapiProperty();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    size_t __stdcall PropCount() const override;

    HRESULT __stdcall GetProperty(size_t index, IProperty** prop) override;
    HRESULT __stdcall SetProperty(size_t index, IProperty* prop) override;

protected:
    ULONG m_cRef;

private:
    struct Impl;
    Impl* pimpl;

private:
    WasapiProperty(const WasapiProperty&);
    WasapiProperty(WasapiProperty&&);
    WasapiProperty& operator =(const WasapiProperty&);
    WasapiProperty& operator =(WasapiProperty&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.WasapiProperty.h