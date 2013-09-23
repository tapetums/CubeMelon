// UI.SimplePlayer.SimplePlayer.cpp

#include <string>
#include <vector>
#include <map>

#include <windows.h>

#include "..\include\ComPtr.h"
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

extern const wchar_t*    MSG_ADD_TO_PLAYLIST = TEXT("UI.SimplePlayer.QuerySupport");

//---------------------------------------------------------------------------//

STDAPI DllGetProperty(size_t index, IPropManager** pm);

//---------------------------------------------------------------------------//

struct COMP_CLASS_NAME::Impl
{
    Impl();
    ~Impl();

    void Clear();

    MainWindow*       main_wnd;
    IOutputComponent* comp_output;

    size_t i_now_playying;
    std::map<std::wstring, ComPtr<IInputComponent>> play_list;
    std::vector<ComPtr<IInputComponent>>  comp_input_array;
    std::vector<ComPtr<IOutputComponent>> comp_output_array;
};

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::Impl::Impl()
{
    main_wnd    = nullptr;
    comp_output = nullptr;

    i_now_playying = 0;
}

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::Impl::~Impl()
{
    this->Clear();
}

//---------------------------------------------------------------------------//

void COMP_CLASS_NAME::Impl::Clear()
{
    play_list.clear();
    comp_input_array.clear();
    comp_output_array.clear();

    if ( comp_output )
    {
        comp_output->Release();
        comp_output = nullptr;
    }
    if ( main_wnd )
    {
        main_wnd->close();
        main_wnd = nullptr;
    }
}

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::COMP_CLASS_NAME(IUnknown* pUnkOuter) : UIComponentBase(pUnkOuter)
{
    DebugPrintLn(TEXT("%s::Constructor() begin"), COMP_NAME);

    pimpl = new Impl;

    DebugPrintLn(TEXT("%s::Constructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

COMP_CLASS_NAME::~COMP_CLASS_NAME()
{
    DebugPrintLn(TEXT("%s::Destructor() begin"), COMP_NAME);

    this->Stop();

    delete pimpl;
    pimpl = nullptr;

    DebugPrintLn(TEXT("%s::Destructor() end"), COMP_NAME);
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Notify(IMsgObject* msg_obj)
{
    DebugPrintLn(TEXT("%s::Notify() begin"), COMP_NAME);

    if ( nullptr == msg_obj )
    {
        DebugPrintLn(TEXT("Message object is void"));
        return E_POINTER;
    }

    HRESULT hr = S_FALSE;

    auto const sender = msg_obj->Sender();
    auto const name   = msg_obj->Name();
    auto const msg    = msg_obj->Message();

    DebugPrintLn(TEXT("%s: %s"), name, msg);

    if ( sender == this )
    {
        if ( lstrcmp(msg, MSG_ADD_TO_PLAYLIST) == 0 )
        {
            IInputComponent* comp_input = nullptr;

            auto const path = (WCHAR*)msg_obj->Data(0);
            auto const itE = pimpl->comp_input_array.end();
            auto it = pimpl->comp_input_array.begin();

            while ( it != itE )
            {
                comp_input = it->GetInterface();
                if ( comp_input )
                {
                    hr = comp_input->QuerySupport(path, TEXT("audio/wav"));
                    if ( SUCCEEDED(hr) )
                    {
                        DebugPrintLn(TEXT(". Adding play list..."));
                        {
                            pimpl->play_list[path] =
                                ComPtr<IInputComponent>(comp_input);
                        }
                        DebugPrintLn(TEXT(". Added play list"));

                        //DebugPrintLn(TEXT(". Opening file..."));
                        {
                            //comp_input->Open(path, TEXT("audio/wav"));
                        }
                        //DebugPrintLn(TEXT(". Opened file"));

                        break;
                    }
                    comp_input = nullptr;
                }
                ++it;
            }
        }
    }

    msg_obj->Release();
    msg_obj = nullptr;

    DebugPrintLn(TEXT("%s::Notify() end"), COMP_NAME);

    return hr;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall COMP_CLASS_NAME::Start(LPVOID args, IComponent* listener)
{
    DebugPrintLn(TEXT("%s::Start() begin"), COMP_NAME);

    if ( m_state & STATE_STARTING )
    {
        DebugPrintLn(TEXT("Now starting"));
        DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);
        return S_FALSE;
    }
    if ( m_state & STATE_ACTIVE )
    {
        DebugPrintLn(TEXT("Already started"));
        DebugPrintLn(TEXT("%s::Start() end"), COMP_NAME);
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    HRESULT hr;

    m_state = (STATE)(m_state | STATE_STARTING);

    ICompManager*    manager    = nullptr;
    ICompAdapter*    adapter    = nullptr;
    IInputComponent* comp_input = nullptr;

    DebugPrintLn(TEXT("Loading input components..."));
    if ( m_owner )
    {
        DebugPrintLn(TEXT("Getting component manager..."));
        hr = m_owner->GetInstance(CLSID_NULL, IID_ICompManager, (void**)&manager);
        DebugPrintLn(TEXT("Got component manager"));
        if ( manager )
        {
            auto count = manager->ComponentCount();
            for ( auto index = 0; index < count; ++index )
            {
                DebugPrintLn(TEXT("Checking whether this is an input component: %d/%d"), index, count);
                adapter = manager->GetAt(index);
                if ( adapter )
                {
                    DebugPrintLn(TEXT("%s"), adapter->Name());
                    if ( IsEqualCLSID(CLSID_Component, adapter->ClassID()) )
                    {
                        DebugPrintLn(TEXT("This is myself"));
                    }
                    else
                    {
                        hr = adapter->CreateInstance
                        (
                            this, IID_IInputComponent,
                            (void**)&comp_input
                        );
                    }
                    adapter->Release();
                    adapter = nullptr;

                    if ( comp_input )
                    {
                        DebugPrintLn(TEXT("This is an input component"));
                        pimpl->comp_input_array.push_back
                        (
                            ComPtr<IInputComponent>(comp_input)
                        );
                        comp_input->Release();
                        comp_input = nullptr;
                    }
                }
            }
            DebugPrintLn(TEXT("Releasing component manager..."));
            manager->Release();
            manager = nullptr;
            DebugPrintLn(TEXT("Released component manager"));
        }
    }
    DebugPrintLn(TEXT("Loaded input components"));

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

HRESULT __stdcall COMP_CLASS_NAME::Stop(IComponent* listener)
{
    DebugPrintLn(TEXT("%s::Stop() begin"), COMP_NAME);

    if ( m_state & STATE_STOPPING )
    {
        DebugPrintLn(TEXT("Now stopping"));
        DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);
        return S_FALSE;
    }
    if ( !(m_state & STATE_ACTIVE) )
    {
        DebugPrintLn(TEXT("Already stopped"));
        DebugPrintLn(TEXT("%s::Stop() end"), COMP_NAME);
        return S_FALSE;
    }

    if ( listener )
    {
        return E_NOTIMPL;
    }

    m_state = (STATE)(m_state | STATE_STOPPING);

    pimpl->Clear();

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