// main.cpp

#include <windows.h>

#include <QApplication>
#include "mainwindow.h"

#include "..\include\DebugPrint.h"
#include "..\include\Interfaces.h"
#include "ComponentManager.h"
#include "Component.h"

//---------------------------------------------------------------------------//

int main(int argc, wchar_t* argv[])
{
    // COMの初期化
    ::CoInitialize(nullptr);

    // アプリケーションインスタンスの生成
    int c = 0;
    QApplication a(c, nullptr);

    // プラグインホストの起動
    auto comp = new Component(nullptr);
    comp->Start();

#if defined(_DEBUG) || defined(DEBUG)
    // メインウィンドウの起動
    auto mwnd = new MainWindow;
    if ( mwnd )
    {
        auto cm = comp->ComponentManager();
        if ( cm )
        {
            IComponentContainer* cc = nullptr;
            auto count = cm->ComponentCount();
            for ( size_t index = 0; index < count; ++index )
            {
                cc = cm->ComponentContainer(index);
                if ( cc )
                {
                    mwnd->addListItem
                    (
                        cc->ClassID(), cc->Name(),
                        cc->Description(), cc->Copyright(), cc->Version()
                    );
                    mwnd->addConsoleText(cc->Name());
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
    comp->Stop();
    comp->Release();
    comp = nullptr;

    // COMの後始末
    ::CoUninitialize();

    return ret;
}

//---------------------------------------------------------------------------//

// main.cpp