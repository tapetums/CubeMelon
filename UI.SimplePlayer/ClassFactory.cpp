// UI.SimplePlayer.ClassFactory.cpp

#include <windows.h>

#include "..\include\LockModule.h"
#include "..\include\ClassFactory.h"
#include "..\include\Interfaces.h"
#include "..\include\Component.h"

//---------------------------------------------------------------------------//

STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** ppvObject)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IClassFactory) )
    {
        *ppvObject = static_cast<IClassFactory*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    this->AddRef();

    return S_OK;
}

//---------------------------------------------------------------------------//

STDMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    this->LockServer(TRUE);

    return 2;
}

//---------------------------------------------------------------------------//

STDMETHODIMP_(ULONG) ClassFactory::Release()
{
    this->LockServer(FALSE);

    return 1;
}

//---------------------------------------------------------------------------//

STDMETHODIMP ClassFactory::CreateInstance
(
     IUnknown* pUnkOuter,
     REFIID    riid,
     void**    ppvObject
)
{
    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    *ppvObject = nullptr;

    IComponent* comp = new Component(pUnkOuter);
    if ( nullptr == comp )
    {
        return E_OUTOFMEMORY;
    }

    hr = comp->QueryInterface(riid, ppvObject);
    if ( SUCCEEDED(hr) && ppvObject )
    {
        hr = comp->Release();
    }

    return hr;
}

//---------------------------------------------------------------------------//

STDMETHODIMP ClassFactory::LockServer(BOOL bLock)
{
    if ( bLock )
    {
        LockModule();
    }
    else
    {
        UnlockModule();
    }

     return S_OK;
}

// UI.SimplePlayer.ClassFactory.cpp