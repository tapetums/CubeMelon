// Trace.h

#pragma once

#include <stdint.h>

//---------------------------------------------------------------------------//

#if 0//defined(_DEBUG) || defined(DEBUG)
// Debugのとき

#include "mainwindow.h"
extern MainWindow* mwnd;

inline void TRACE(LPCWSTR t)
{
    if ( mwnd )
    {
        mwnd->addConsoleText(t);
    }
}

inline void TRACE(LPCWSTR t, int64_t a, int64_t b = 0, int64_t c = 0, int64_t d = 0)
{
    if ( mwnd )
    {
        mwnd->addConsoleText(t, a, b, c, d);
    }
}

#else
// Releaseのとき

#include <QString>
inline void TRACE(LPCWSTR t, int64_t a = 0, int64_t b = 0, int64_t c = 0, int64_t d = 0)
{
}

#endif

// Trace.h