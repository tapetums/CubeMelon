// SimplePlayer.DllMain.cpp

#include <windows.h>
#include <propsys.h>

#include "..\include\LockModule.h"
#include "..\include\ClassFactory.h"
#include "..\include\PluginProperty.h"

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Plugin;

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

STDAPI DllGetProperty(size_t index, IPropertyStore** ps)
{
    if ( nullptr == ps )
    {
        return E_INVALIDARG;
    }

    if ( index == 0 )
    {
        static PluginProperty prop;
        *ps = &prop;
        (*ps)->AddRef();

        return S_OK;
    }
    else
    {
        *ps = nullptr;

        return CLASS_E_CLASSNOTAVAILABLE;
    }
}

//---------------------------------------------------------------------------//

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObject)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if ( !IsEqualCLSID(rclsid, CLSID_Plugin) )
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

// SimplePlayer.DllMain.cpp