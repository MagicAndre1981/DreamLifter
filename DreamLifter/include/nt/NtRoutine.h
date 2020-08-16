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

// MmGetSystemRoutineAddress
PVOID DlkMmGetSystemRoutineAddress(
	PUNICODE_STRING SystemRoutineName
);

// PsGetVersion
BOOLEAN DlkPsGetVersion(
	PULONG          MajorVersion,
	PULONG          MinorVersion,
	PULONG          BuildNumber,
	PUNICODE_STRING CSDVersion
);

typedef ULONG64 TRACEHANDLE, * PTRACEHANDLE;

// WmiTraceMessage
NTSTATUS DlkWmiTraceMessage(
	TRACEHANDLE LoggerHandle,
	ULONG       MessageFlags,
	LPCGUID     MessageGuid,
	USHORT      MessageNumber,
	...
);

// WmiQueryTraceInformation
typedef enum _TRACE_INFORMATION_CLASS {
	TraceIdClass,
	TraceHandleClass,
	TraceEnableFlagsClass,
	TraceEnableLevelClass,
	GlobalLoggerHandleClass,
	EventLoggerHandleClass,
	AllLoggerHandlesClass,
	TraceHandleByNameClass,
	LoggerEventsLostClass,
	TraceSessionSettingsClass,
	LoggerEventsLoggedClass,
	DiskIoNotifyRoutinesClass,
	TraceInformationClassReserved1,
	FltIoNotifyRoutinesClass,
	TraceInformationClassReserved2,
	WdfNotifyRoutinesClass,
	MaxTraceInformationClass
} TRACE_INFORMATION_CLASS;

NTSTATUS DlkWmiQueryTraceInformation(
	TRACE_INFORMATION_CLASS TraceInformationClass,
	PVOID                   TraceInformation,
	ULONG                   TraceInformationLength,
	PULONG                  RequiredLength,
	PVOID                   Buffer
);

// EtwRegisterClassicProvider
typedef ULONGLONG REGHANDLE, * PREGHANDLE;

NTSTATUS DlkEtwRegisterClassicProvider(
	LPCGUID ProviderId,
	ULONG Type,
	PVOID EnableCallback,
	PVOID CallbackContext,
	REGHANDLE* RegHandle
);

// EtwUnregister
NTSTATUS DlkEtwUnregister(
	REGHANDLE RegHandle
);

// IoWMIRegistrationControl
NTSTATUS DlkIoWMIRegistrationControl(
	PVOID		   DeviceObject,
	ULONG          Action
);

// DbgPrint
ULONG DlkDbgPrint(
	PCSTR Format,
	...
);
