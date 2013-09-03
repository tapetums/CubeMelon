// main.cpp

#include <windows.h>

#include <QApplication>
#include "mainwindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\Interfaces.h"
#include "PluginManager.h"
#include "Host.h"

//---------------------------------------------------------------------------//

int main(int argc, wchar_t* argv[])
{
    // COMの初期化
    ::CoInitialize(nullptr);

    // アプリケーションインスタンスの生成
    int c = 0;
    QApplication a(c, nullptr);

    // プラグインホストの起動
    auto host = new Host(nullptr);
    host->Start();

#if defined(_DEBUG) || defined(DEBUG)
    // メインウィンドウの起動
    auto mwnd = new MainWindow;
    if ( mwnd )
    {
        auto pm = host->PluginManager();
        if ( pm )
        {
            IPluginContainer* pc = nullptr;
            auto count = pm->PluginCount();
            for ( size_t index = 0; index < count; ++index )
            {
                pc = pm->PluginContainer(index);
                if ( pc )
                {
                    mwnd->addListItem(pc->Name(), pc->ClassID());
                    mwnd->addConsoleText(pc->Name());
                }
            }
        }
        mwnd->addConsoleText(TEXT("Ready"));
        mwnd->showMinimized ();
    }
#endif

    // メッセージループ
    DebugPrintLn(TEXT("---------------- Message Loop Begin ----------------"));

    int ret = a.exec();

    DebugPrintLn(TEXT("----------------  Message Loop End  ----------------"));

#if defined(_DEBUG) || defined(DEBUG)
    // メインウィンドウの破棄
    if ( mwnd )
    {
        delete mwnd;
        mwnd = nullptr;
    }
#endif

    // プラグインホストの終了
    host->Stop();
    host->Release();
    host = nullptr;

    // COMの後始末
    ::CoUninitialize();

    return ret;
}

//---------------------------------------------------------------------------//

// main.cpp