// main.cpp

#include <windows.h>

#include "..\include\DebugPrint.h"
#include "..\include\Interfaces.h"

#include "Host.h"

#include <QApplication>

//---------------------------------------------------------------------------//

int main()
{
    // COMの初期化
    ::CoInitialize(nullptr);

    // アプリケーションインスタンスの生成
    int c = 0;
    QApplication a(c, nullptr);

    // ホストコンポーネントの起動
    auto host = new CubeMelon::Host(nullptr);
    host->Start();

    // メッセージループ
    DebugPrintLn(TEXT("---------------- Message Loop Begin ----------------"));
    auto ret = a.exec();
    DebugPrintLn(TEXT("----------------  Message Loop End  ----------------"));

    // ホストコンポーネントの終了
    host->Stop();
    host->Release();
    host = nullptr;

    // COMの後始末
    ::CoUninitialize();

    return ret;
}

//---------------------------------------------------------------------------//

// main.cpp