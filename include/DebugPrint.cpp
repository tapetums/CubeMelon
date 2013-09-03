// DebugPrint.cpp

#include "DebugPrint.h"

#if defined(_DEBUG) || defined(DEBUG)
// Debugのとき

#include <windows.h>
#include <strsafe.h>

//---------------------------------------------------------------------------//

#define BUFSIZE 1024

//---------------------------------------------------------------------------//

void DebugPrint(const wchar_t* format, ...)
{
    wchar_t buf[BUFSIZE];

    SYSTEMTIME st;
    ::GetLocalTime(&st);
    ::StringCchPrintf
    (
        buf, BUFSIZE,
        TEXT("%02d:%02d:%02d;%04d> "),
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds
    );
    ::OutputDebugStringW(buf);

    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfW(buf, BUFSIZE, format, al);
    }
    va_end(al);
    ::OutputDebugStringW(buf);
}

//---------------------------------------------------------------------------//

void DebugPrintLn(const wchar_t* format, ...)
{
    DebugPrint(format);

    ::OutputDebugStringW(L"\n");
}

//---------------------------------------------------------------------------//

#endif

// DebugPrint.cpp