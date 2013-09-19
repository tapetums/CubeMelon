// Functions.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon
{
    struct IPropManager;
}

//---------------------------------------------------------------------------//

extern "C"
{
    typedef HRESULT (__stdcall* DLLCANUNLOADNOW)();
    typedef HRESULT (__stdcall* DLLCONFIGURE)(HWND);
    typedef HRESULT (__stdcall* DLLGETCLASSOBJECT)(REFCLSID, REFIID, void**);
    typedef HRESULT (__stdcall* DLLGETPROPMANAGER)(size_t, CubeMelon::IPropManager**);
}

//---------------------------------------------------------------------------//

// Functions.h