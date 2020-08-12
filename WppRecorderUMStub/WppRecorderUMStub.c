// WppRecorderUMStub.c : Defines the exported functions for the DLL.

#include <Windows.h>
#include "framework.h"
#include "WppRecorderUMStub.h"

WPPRECORDERUMSTUB_API void WppAutoLogStop()
{
	OutputDebugString(L"[STUB] Calling WppAutoLogStop\n");
}

WPPRECORDERUMSTUB_API void WppAutoLogStart()
{
	OutputDebugString(L"[STUB] Calling WppAutoLogStart\n");
}

WPPRECORDERUMSTUB_API void WppAutoLogTrace()
{
	OutputDebugString(L"[STUB] Calling WppAutoLogTrace\n");
}
