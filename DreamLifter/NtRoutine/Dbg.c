// Dbg.c: Debug utilities

#include <DreamLifter.h>

ULONG DlkDbgPrint(
	PCSTR Format,
	...
)
{
	ULONG i = 0;

	va_list Args;
	va_start(Args, Format);
	i = vprintf(Format, Args);
	va_end(Args);

	return i;
}
