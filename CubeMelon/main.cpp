#include <windows.h>

#include <QApplication>
#include "mainwindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\Interfaces.h"
#include "..\include\Plugin.h"
#include "..\include\PluginManager.h"

//---------------------------------------------------------------------------//

extern MainWindow* mwnd = nullptr;

//---------------------------------------------------------------------------//

int main(int argc, wchar_t* argv[])
{
    // COMの初期化
    ::CoInitialize(nullptr);

    // アプリケーションインスタンスの生成
    int c = 0;
    QApplication a(c, nullptr);

    // メインウィンドウの起動
#if defined(_DEBUG) || defined(DEBUG)
    mwnd = new MainWindow;
#endif
    if ( mwnd )
    {
       mwnd->showMinimized ();
    }

    // プラグインホストの起動
    auto host = new Plugin(nullptr);
    host->Start();

    if ( mwnd )
    {
        ::PluginManager* pm = nullptr;
        auto hr = host->Notify(nullptr, TEXT("GetPluginManager"), &pm, 0);
        if ( SUCCEEDED(hr) && nullptr != pm )
        {
            auto pi = pm->AllPlugins();
            auto count = pm->PluginCount();
            for ( size_t index = 0; index < count; ++index )
            {
                mwnd->addListItem(pi[index]->Name(), pi[index]->ClassID());
                mwnd->addConsoleText(pi[index]->Name());
            }
        }

        mwnd->addConsoleText(TEXT("Ready"));
    }

    // メッセージループ
    DebugPrintLn(TEXT("---------------- Message Loop Begin ----------------"));
    int ret = a.exec();
    DebugPrintLn(TEXT("----------------  Message Loop End  ----------------"));

    // プラグインホストの終了
    host->Stop();
    host->Release();
    host = nullptr;

    // メインウィンドウの破棄
    if ( mwnd )
    {
        delete mwnd;
        mwnd = nullptr;
    }

    // COMの後始末
    ::CoUninitialize();

    return ret;
}
