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
DWORD WINAPI DlPoSettingsCallback(PVOID Context, ULONG Type, PVOID Setting);

typedef struct _DL_POWERSETTINGS_CALLBACK_CONTEXT {
	PPOWER_SETTING_CALLBACK DriverCallback;
	PVOID DriverContext;
	GUID PowerSettingsGuid;
} DL_POWERSETTINGS_CALLBACK_CONTEXT, *PDL_POWERSETTINGS_CALLBACK_CONTEXT;

NTSTATUS DlPoRegisterPowerSettingCallback(
	PVOID					DeviceObject,
	LPCGUID                 SettingGuid,
	PPOWER_SETTING_CALLBACK Callback,
	PVOID                   Context,
	PVOID * Handle
);

// ZwCreateFile
NTSTATUS DlZwCreateFile(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	PLARGE_INTEGER     AllocationSize,
	ULONG              FileAttributes,
	ULONG              ShareAccess,
	ULONG              CreateDisposition,
	ULONG              CreateOptions,
	PVOID              EaBuffer,
	ULONG              EaLength
);

// ZwClose
NTSTATUS DlZwClose(
	HANDLE Handle
);

// Mutex and event
#define TIMER_TOLERABLE_DELAY_BITS      6
#define TIMER_EXPIRED_INDEX_BITS        6
#define TIMER_PROCESSOR_INDEX_BITS      5

#pragma warning(disable:4214)
#pragma warning(disable:4201)
typedef struct _DISPATCHER_HEADER {
	union {
		union {
			volatile LONG Lock;
			LONG LockNV;
		} DUMMYUNIONNAME;

		struct {                            // Events, Semaphores, Gates, etc.
			UCHAR Type;                     // All (accessible via KOBJECT_TYPE)
			UCHAR Signalling;
			UCHAR Size;
			UCHAR Reserved1;
		} DUMMYSTRUCTNAME;

		struct {                            // Timer
			UCHAR TimerType;
			union {
				UCHAR TimerControlFlags;
				struct {
					UCHAR Absolute : 1;
					UCHAR Wake : 1;
					UCHAR EncodedTolerableDelay : TIMER_TOLERABLE_DELAY_BITS;
				} DUMMYSTRUCTNAME;
			};

			UCHAR Hand;
			union {
				UCHAR TimerMiscFlags;
				struct {
					UCHAR Index : TIMER_EXPIRED_INDEX_BITS;
					UCHAR Inserted : 1;
					volatile UCHAR Expired : 1;
				} DUMMYSTRUCTNAME;
			} DUMMYUNIONNAME;
		} DUMMYSTRUCTNAME2;

		struct {                            // Timer2
			UCHAR Timer2Type;
			union {
				UCHAR Timer2Flags;
				struct {
					UCHAR Timer2Inserted : 1;
					UCHAR Timer2Expiring : 1;
					UCHAR Timer2CancelPending : 1;
					UCHAR Timer2SetPending : 1;
					UCHAR Timer2Running : 1;
					UCHAR Timer2Disabled : 1;
					UCHAR Timer2ReservedFlags : 2;
				} DUMMYSTRUCTNAME;
			} DUMMYUNIONNAME;

			UCHAR Timer2ComponentId;
			UCHAR Timer2RelativeId;
		} DUMMYSTRUCTNAME3;

		struct {                            // Queue
			UCHAR QueueType;
			union {
				UCHAR QueueControlFlags;
				struct {
					UCHAR Abandoned : 1;
					UCHAR DisableIncrement : 1;
					UCHAR QueueReservedControlFlags : 6;
				} DUMMYSTRUCTNAME;
			} DUMMYUNIONNAME;

			UCHAR QueueSize;
			UCHAR QueueReserved;
		} DUMMYSTRUCTNAME4;

		struct {                            // Thread
			UCHAR ThreadType;
			UCHAR ThreadReserved;

			union {
				UCHAR ThreadControlFlags;
				struct {
					UCHAR CycleProfiling : 1;
					UCHAR CounterProfiling : 1;
					UCHAR GroupScheduling : 1;
					UCHAR AffinitySet : 1;
					UCHAR Tagged : 1;
					UCHAR EnergyProfiling : 1;
					UCHAR SchedulerAssist : 1;
					UCHAR ThreadReservedControlFlags : 1;
				} DUMMYSTRUCTNAME;
			} DUMMYUNIONNAME;

			union {
				UCHAR DebugActive;
				struct {
					BOOLEAN ActiveDR7 : 1;
					BOOLEAN Instrumented : 1;
					BOOLEAN Minimal : 1;
					BOOLEAN Reserved4 : 3;
					BOOLEAN UmsScheduled : 1;
					BOOLEAN UmsPrimary : 1;
				} DUMMYSTRUCTNAME;
			} DUMMYUNIONNAME2;
		} DUMMYSTRUCTNAME5;

		struct {                         // Mutant
			UCHAR MutantType;
			UCHAR MutantSize;
			BOOLEAN DpcActive;
			UCHAR MutantReserved;
		} DUMMYSTRUCTNAME6;
	} DUMMYUNIONNAME;

	LONG SignalState;                   // Object lock
	LIST_ENTRY WaitListHead;            // Object lock
} DISPATCHER_HEADER, * PDISPATCHER_HEADER;

typedef struct _KEVENT {
	DISPATCHER_HEADER Header;
} KEVENT, * PKEVENT, * PRKEVENT;

typedef struct _KMUTANT {
	DISPATCHER_HEADER Header;
	LIST_ENTRY MutantListEntry;
	PVOID OwnerThread;

	union {
		UCHAR MutantFlags;
		struct {
			UCHAR Abandoned : 1;
			UCHAR Spare1 : 7;
		} DUMMYSTRUCTNAME;
	} DUMMYUNIONNAME;

	UCHAR ApcDisable;
} KMUTANT, * PKMUTANT, * PRKMUTANT, KMUTEX, * PKMUTEX, * PRKMUTEX;
#pragma warning(default:4214)
#pragma warning(default:4201)

void DlKeInitializeMutex(
	PRKMUTEX Mutex,
	ULONG    Level
);

LONG DlKeReleaseMutex(
	PRKMUTEX Mutex,
	BOOLEAN  Wait
);

typedef enum _EVENT_TYPE {
	NotificationEvent,
	SynchronizationEvent
} EVENT_TYPE;

void DlKeInitializeEvent(
	PRKEVENT   Event,
	EVENT_TYPE Type,
	BOOLEAN    State
);

void DlKeClearEvent(
	PRKEVENT Event
);

LONG DlKeSetEvent(
	PRKEVENT  Event,
	KPRIORITY Increment,
	BOOLEAN   Wait
);

LONG DlKeReadStateEvent(
	PRKEVENT Event
);

typedef enum _WAIT_TYPE {
	WaitAll,
	WaitAny,
	WaitNotification,
	WaitDequeue
} WAIT_TYPE;

typedef enum _KWAIT_REASON {
	Executive,
	FreePage,
	PageIn,
	PoolAllocation,
	DelayExecution,
	Suspended,
	UserRequest,
	WrExecutive,
	WrFreePage,
	WrPageIn,
	WrPoolAllocation,
	WrDelayExecution,
	WrSuspended,
	WrUserRequest,
	WrSpare0,
	WrQueue,
	WrLpcReceive,
	WrLpcReply,
	WrVirtualMemory,
	WrPageOut,
	WrRendezvous,
	WrKeyedEvent,
	WrTerminated,
	WrProcessInSwap,
	WrCpuRateControl,
	WrCalloutStack,
	WrKernel,
	WrResource,
	WrPushLock,
	WrMutex,
	WrQuantumEnd,
	WrDispatchInt,
	WrPreempted,
	WrYieldExecution,
	WrFastMutex,
	WrGuardedMutex,
	WrRundown,
	WrAlertByThreadId,
	WrDeferredPreempt,
	WrPhysicalFault,
	MaximumWaitReason
} KWAIT_REASON;

typedef CCHAR KPROCESSOR_MODE;

typedef enum _MODE {
	KernelMode,
	UserMode,
	MaximumMode
} MODE;


NTSTATUS
DlKeWaitForSingleObject(
	PVOID Object,
	KWAIT_REASON WaitReason,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
);
