// Dbg.c: Debug utilities

#include <DreamLifter.h>

ULONG DlkDbgPrint(
	PCSTR Format,
	...
)
{
	ULONG i = 0;
	char DebugBuffer[512];

	va_list Args;
	va_start(Args, Format);

	i = vprintf(Format, Args);
	if (sprintf_s(DebugBuffer, 512, Format, Args))
	{
		OutputDebugStringA(DebugBuffer);
	}

	va_end(Args);
	return i;
}
