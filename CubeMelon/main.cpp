// main.cpp

#include <windows.h>

#include <objbase.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
#include "..\include\ClassFactory.h"
#include "..\include\Interfaces.h"
#include "..\include\ComponentBase.h"
#include "..\include\PropManager.h"

#include "Host.h"

#include <QApplication>

//---------------------------------------------------------------------------//

namespace CubeMelon
{
    extern const CLSID CLSID_Component;
}

//---------------------------------------------------------------------------//

int main()
{
    DebugPrintLn(TEXT("**************************************************"));

    // COMの初期化
    ::CoInitialize(nullptr);

    // アプリケーションインスタンスの生成
    int c = 0;
    QApplication a(c, nullptr);

    // ホストコンポーネントの起動
    auto host = new CubeMelon::Host;
    host->Start();

    // メッセージループ
    DebugPrintLn(TEXT("---------------- Message Loop Begin ----------------"));
    auto ret = a.exec();
    DebugPrintLn(TEXT("----------------  Message Loop End  ----------------"));

    // たまにUIスレッドの破棄が間に合わないので少し待つ
    ::Sleep(100);

    // ホストコンポーネントの終了
    host->Release();
    host = nullptr;

    // COMの後始末
    ::CoUninitialize();

    DebugPrintLn(TEXT("**************************************************"));

    if ( g_cLocks )
    {
        DebugPrintLn(TEXT("\tMemory leaked!!!    %d"), g_cLocks);
    }
    else
    {
        DebugPrintLn(TEXT("\tNormal termination    %d"), g_cLocks);
    }

    return ret;
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

// main.cpp