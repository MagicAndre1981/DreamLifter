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

// C Routines
size_t __cdecl Dlkwcslen(
	const wchar_t* str
);

size_t __cdecl Dlkstrlen(
	_In_z_ char const* _Str
);

char* Dlkstrcpy(
	char* strDestination,
	const char* strSource
);

_Success_(return >= 0)
_Check_return_opt_
int __cdecl Dlksprintf_s(
	_Out_writes_(_BufferCount) _Always_(_Post_z_) char* const _Buffer,
	_In_                                          size_t      const _BufferCount,
	_In_z_ _Printf_format_string_                 char const* const _Format,
	...
);

_Success_(return >= 0)
_Check_return_opt_
int _cdecl Dlksprintf(
	_Pre_notnull_ _Always_(_Post_z_) char* const _Buffer,
	_In_z_ _Printf_format_string_    char const* const _Format,
	...
);

_Success_(return >= 0)
_Check_return_opt_
int _cdecl Dlk_vsnwprintf(
	_Out_writes_opt_(_BufferCount) _Post_maybez_ wchar_t* _Buffer,
	_In_                                         size_t         _BufferCount,
	_In_z_ _Printf_format_string_                wchar_t const* _Format,
	va_list        _ArgList
);

_Success_(return >= 0)
_Check_return_opt_
int _cdecl Dlk_vsnprintf(
	_Out_writes_opt_(_BufferCount) _Post_maybez_ char* const _Buffer,
	_In_                                        size_t      const _BufferCount,
	_In_z_ _Printf_format_string_               char const* const _Format,
	va_list           _ArgList
);

UINT8 DlKeGetCurrentIrql();

// PoRegisterPowerSettingCallback
typedef
_IRQL_requires_same_
NTSTATUS
POWER_SETTING_CALLBACK(
	_In_ LPCGUID SettingGuid,
	_In_reads_bytes_(ValueLength) PVOID Value,
	_In_ ULONG ValueLength,
	_Inout_opt_ PVOID Context
);

typedef POWER_SETTING_CALLBACK* PPOWER_SETTING_CALLBACK;

NTSTATUS DlPoRegisterPowerSettingCallback(
	PVOID					DeviceObject,
	LPCGUID                 SettingGuid,
	PPOWER_SETTING_CALLBACK Callback,
	PVOID                   Context,
	PVOID * Handle
);
