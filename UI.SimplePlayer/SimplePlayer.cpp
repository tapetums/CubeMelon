// UI.SimplePlayer.SimplePlayer.cpp

#include <windows.h>

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\Interfaces.h"
#include "..\include\ComponentBase.h"

#include "mainwindow.h"

#include "SimplePlayer.h"

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID CLSID_Component =
{ 0x520e13ad, 0x3345, 0x4377, { 0xb2, 0xef, 0x68, 0x42, 0xea, 0x79, 0xb2, 0x5b } };

extern const size_t      MDL_PROP_COUNT    = 5;
extern const wchar_t*    MDL_PROP_MGR_NAME = TEXT("UI.SimplePlayer.PropManager");
extern const wchar_t*    MDL_PROP_NAME     = TEXT("UI.SimplePlayer.Property");

extern const size_t      COMP_INDEX      = 0;
extern const wchar_t*    COMP_NAME       = TEXT("UI.SimplePlayer");

extern const wchar_t*    PropName        = TEXT("UI.SimplePlayer");
extern const wchar_t*    PropDescription = TEXT("Audio player window component for CubeMelon");
extern const wchar_t*    PropCopyright   = TEXT("(C) 2012-2013 tapetums");
extern const VersionInfo PropVersion     = { 1, 0, 0, 0 };

//---------------------------------------------------------------------------//

STDAPI DllGetProperty(size_t index, IPropManager** pm);

//---------------------------------------------------------------------------//

struct SimplePlayer::Impl
{
    Impl();
    ~Impl();

    MainWindow* main_wnd;
    IComponent* comp_input;
    IComponent* comp_output;
};

//---------------------------------------------------------------------------//

SimplePlayer::Impl::Impl()
{
    main_wnd    = nullptr;
    comp_input  = nullptr;
    comp_output = nullptr;
}

//---------------------------------------------------------------------------//

SimplePlayer::Impl::~Impl()
{
    if ( comp_input )
    {
        comp_input->Release();
        comp_input = nullptr;
    }
    if ( comp_output )
    {
        comp_output->Release();
        comp_output = nullptr;
    }
    if ( main_wnd )
    {
        delete main_wnd;
        main_wnd = nullptr;
    }
}

//---------------------------------------------------------------------------//

SimplePlayer::SimplePlayer(IUnknown* pUnkOuter) : UIComponentBase(pUnkOuter)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_NAME);

    pimpl = new Impl;

    DebugPrintLn(TEXT("%s::Constructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

SimplePlayer::~SimplePlayer()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), COMP_NAME);

    this->Stop();

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(TEXT("%s::Destructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::Start(LPVOID args, IComponent* listener)
{
    DebugPrintLn(TEXT("%s::Start() begin"), COMP_NAME);

    if ( m_state & STATE_STARTING )
    {
        DebugPrintLn(TEXT("Now starting"));
        return S_FALSE;
    }
    if ( m_state & STATE_ACTIVE )
    {
        DebugPrintLn(TEXT("Already started"));
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STARTING);

    pimpl->main_wnd = new MainWindow;
    pimpl->main_wnd->setOwner(static_cast<IComponent*>((void*)this));
    pimpl->main_wnd->show();

    m_hwnd = reinterpret_cast<HWND>(pimpl->main_wnd->winId());

    m_state = (STATE)(m_state | STATE_ACTIVE);
    m_state = (STATE)(m_state ^ STATE_STARTING);

    DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SimplePlayer::Stop(IComponent* listener)
{
    DebugPrintLn(TEXT("%s::Stop() begin"), COMP_NAME);

    if ( m_state & STATE_STOPPING )
    {
        DebugPrintLn(TEXT("Now stopping"));
        return S_FALSE;
    }
    if ( !(m_state & STATE_ACTIVE) )
    {
        DebugPrintLn(TEXT("Already stopped"));
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STOPPING);

    if ( pimpl->main_wnd )
    {
        delete pimpl->main_wnd;
        pimpl->main_wnd = nullptr;
    }
    if ( pimpl->comp_input )
    {
        pimpl->comp_input->Release();
        pimpl->comp_input = nullptr;
    }
    if ( pimpl->comp_output )
    {
        pimpl->comp_output->Release();
        pimpl->comp_output = nullptr;
    }
    if ( m_owner )
    {
        auto hr = m_owner->Stop();
        if ( S_OK != hr )
        {
            DebugPrintLn(TEXT("Failed to Stop %s's Owner"), COMP_NAME);
        }
    }

    m_state = (STATE)(m_state ^ STATE_ACTIVE);
    m_state = (STATE)(m_state ^ STATE_STOPPING);

    DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// UI.SimplePlayer.SimplePlayer.cpp