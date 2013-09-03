﻿// LockModule.cpp

#include <windows.h>

#include "LockModule.h"

#include "DebugPrint.h"

//---------------------------------------------------------------------------//

ULONG g_cLocks = 0;

//---------------------------------------------------------------------------//

void LockModule()
{
     ::InterlockedIncrement(&g_cLocks);

     DebugPrintLn(TEXT("LockModule() %d"), g_cLocks);
}

//---------------------------------------------------------------------------//

void UnlockModule()
{
    ::InterlockedDecrement(&g_cLocks);

     DebugPrintLn(TEXT("UnlockModule() %d"), g_cLocks);
}

// LockModule.cpp