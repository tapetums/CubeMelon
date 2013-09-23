// main.cpp

#include <windows.h>

#include <objbase.h>
#pragma comment(lib, "ole32.lib")

#include "..\include\DebugPrint.h"
#include "..\include\LockModule.h"
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
    QApplication app(c, nullptr);

    // ホストコンポーネントの起動
    auto host = new CubeMelon::Host;
    host->Start();

    // メッセージループ
    DebugPrintLn(TEXT(""));
    DebugPrintLn(TEXT("---------------- Message Loop ----------------"));
    DebugPrintLn(TEXT(""));
    auto ret = app.exec();
    DebugPrintLn(TEXT(""));
    DebugPrintLn(TEXT("---------------- Message Loop ----------------"));
    DebugPrintLn(TEXT(""));

    // ホストコンポーネントの終了
    host->Stop();
    host->Release();
    host = nullptr;

    // たまにUIスレッドの破棄が間に合わないので少し待つ。これはQt側の問題
    ::Sleep(1000);

    // COMの後始末
    ::CoUninitialize();

    DebugPrintLn(TEXT("**************************************************"));
    DebugPrintLn(TEXT("\tRemaining g_cLock: %05d"), g_cLocks);

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
    return E_NOTIMPL;
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