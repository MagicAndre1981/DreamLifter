// Rtl.c: ntoskrnl Rtl-related wrapper

#include <DreamLifter.h>

// RtlCopyUnicodeString
VOID DlkRtlCopyUnicodeString(
	PUNICODE_STRING  DestinationString,
	PCUNICODE_STRING SourceString
)
{
	USHORT l = 0;

	// If SourceString is NULL, this routine sets the Length field of the structure pointed to by DestinationString to zero.
	if (SourceString == NULL) {
		DestinationString->Length = 0;
		return;
	}

	// The number of bytes copied from the source string is either the source string length
	// or the maximum length of the destination string, whichever is smaller.
	l = min(DestinationString->MaximumLength, SourceString->Length);
	RtlCopyMemory(DestinationString->Buffer, SourceString->Buffer, l);
}

// RtlInitUnicodeString
VOID DlKRtlInitUnicodeString(
	PUNICODE_STRING DestinationString,
	PCWSTR SourceString
)
{
	RtlInitUnicodeString(DestinationString, SourceString);
}

// RtlCompareMemory
SIZE_T DlkRtlCompareMemory(
	const VOID* Source1,
	const VOID* Source2,
	SIZE_T     Length
)
{
	return RtlCompareMemory(Source1, Source2, Length);
}
