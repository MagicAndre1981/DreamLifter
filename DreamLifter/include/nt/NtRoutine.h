#pragma once

// RtlCopyUnicodeString
VOID DlkRtlCopyUnicodeString(
	PUNICODE_STRING  DestinationString,
	PCUNICODE_STRING SourceString
);

// RtlInitUnicodeString
VOID DlKRtlInitUnicodeString(
	PUNICODE_STRING DestinationString,
	PCWSTR SourceString
);

// RtlCompareMemory
SIZE_T DlkRtlCompareMemory(
	const VOID* Source1,
	const VOID* Source2,
	SIZE_T     Length
);
