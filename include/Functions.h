// Functions.h

#pragma once

//---------------------------------------------------------------------------//

extern "C"
{
    typedef HRESULT (__stdcall* DLLCANUNLOADNOW)();
    typedef HRESULT (__stdcall* DLLCONFIGURE)(HWND);
    typedef HRESULT (__stdcall* DLLGETPROPERTY)(size_t, IPropertyStore**);
    typedef HRESULT (__stdcall* DLLGETCLASSOBJECT)(REFCLSID, REFIID, void**);
}

//---------------------------------------------------------------------------//

// Functions.h