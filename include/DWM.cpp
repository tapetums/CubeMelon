#include <windows.h>
#include <dwmapi.h>

#include <QWidget>

#include "DWM.h"

//---------------------------------------------------------------------------//

#pragma warning (disable : 4533 )

//---------------------------------------------------------------------------//

HRESULT EnableAeroGlass(QWidget* widget)
{
    if ( nullptr == widget )
    {
        return E_POINTER;
    }

    HRESULT hr = E_FAIL;

    auto dwmapi = ::LoadLibraryExW
    (
        TEXT("dwmapi.dll"), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH
    );
    if ( nullptr == dwmapi )
    {
        goto FREE_DLL;
    }

    auto DwmIsCompositionEnabled = (HRESULT (__stdcall*)(BOOL*))::GetProcAddress
    (
        dwmapi, "DwmIsCompositionEnabled"
    );
    if ( nullptr == DwmIsCompositionEnabled )
    {
        goto FREE_DLL;
    }

    auto DwmEnableBlurBehindWindow = (HRESULT (__stdcall*)(HWND, const DWM_BLURBEHIND*))::GetProcAddress
    (
        dwmapi, "DwmEnableBlurBehindWindow"
    );
    if ( nullptr == DwmEnableBlurBehindWindow )
    {
        goto FREE_DLL;
    }

    auto DwmExtendFrameIntoClientArea = (HRESULT (__stdcall*)(HWND, const MARGINS*))::GetProcAddress
    (
        dwmapi, "DwmExtendFrameIntoClientArea"
    );
    if ( nullptr == DwmExtendFrameIntoClientArea )
    {
        goto FREE_DLL;
    }

    BOOL fEnable = FALSE;
    hr = DwmIsCompositionEnabled(&fEnable);
    if ( FAILED(hr) || !fEnable )
    {
        goto FREE_DLL;
    }

    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setAttribute(Qt::WA_NoSystemBackground);

    auto hwnd = reinterpret_cast<HWND>(widget->winId());

    DWM_BLURBEHIND bb = { };
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = nullptr;
    hr = DwmEnableBlurBehindWindow(hwnd, &bb);
    if( S_OK != hr )
    {
        goto FREE_DLL;
    }

    const MARGINS margins = { -1 };
    hr = DwmExtendFrameIntoClientArea(hwnd, &margins);

    FREE_DLL:
    if ( dwmapi )
    {
        ::FreeLibrary(dwmapi);
        dwmapi = nullptr;
    }

    return hr;
}
