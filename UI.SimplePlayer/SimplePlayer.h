// UI.SimplePlayer.SimplePlayer.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class SimplePlayer : public UIComponentBase
{
public:
    explicit SimplePlayer(IUnknown* pUnkOuter);
    ~SimplePlayer();

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