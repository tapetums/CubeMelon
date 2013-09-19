// Output.Wasapi.DllMain.cpp

#include <windows.h>

#include "..\include\LockModule.h"
#include "..\include\ClassFactory.h"
#include "..\include\Interfaces.h"
#include "..\include\PropManager.h"

//---------------------------------------------------------------------------//

namespace CubeMelon
{
    extern const CLSID CLSID_Component;
}

//---------------------------------------------------------------------------//

HINSTANCE g_hInst = nullptr;

//---------------------------------------------------------------------------//

BOOL __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    g_hInst = hinstDLL;

    if ( fdwReason == DLL_PROCESS_ATTACH )
    {
        g_hInst = hinstDLL;
        ::DisableThreadLibraryCalls(hinstDLL);
    }
    else if ( fdwReason == DLL_PROCESS_DETACH )
    {
    }

    return TRUE;
}

//---------------------------------------------------------------------------//

STDAPI DllCanUnloadNow()
{
    return (g_cLocks == 0) ? S_OK : S_FALSE;
}

//---------------------------------------------------------------------------//

STDAPI DllConfigure(HWND hwndParent = nullptr)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObject)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if ( !IsEqualCLSID(rclsid, CubeMelon::CLSID_Component) )
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    if ( IsEqualIID(riid, IID_IClassFactory) )
    {
        static ClassFactory factory;

        return factory.QueryInterface(riid, ppvObject);
    }
    else
    {
        return E_NOINTERFACE;
    }
}

//---------------------------------------------------------------------------//

STDAPI DllGetPropManager(size_t index, CubeMelon::IPropManager** pm)
{
    if ( nullptr == pm )
    {
        return E_INVALIDARG;
    }

    if ( index == 0 )
    {
        *pm = new CubeMelon::PropManager;
        return S_OK;
    }
    else
    {
        *pm = nullptr;
        return CLASS_E_CLASSNOTAVAILABLE;
    }
}

//---------------------------------------------------------------------------//

// Output.Wasapi.DllMain.cpp