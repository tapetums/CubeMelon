// CubeMelon.Host.h

#pragma once

//---------------------------------------------------------------------------//

#define COMP_CLASS_NAME Host

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class COMP_CLASS_NAME : public ComponentBase
{
public:
    COMP_CLASS_NAME();
    ~COMP_CLASS_NAME();

    HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// CubeMelon.Host.h