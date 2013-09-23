// UI.SimplePlayer.SimplePlayer.h

#pragma once

//---------------------------------------------------------------------------//

#define COMP_CLASS_NAME SimplePlayer

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class COMP_CLASS_NAME : public UIComponentBase
{
public:
    explicit COMP_CLASS_NAME(IUnknown* pUnkOuter);
    ~COMP_CLASS_NAME();

    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// UI.SimplePlayer.SimplePlayer.h