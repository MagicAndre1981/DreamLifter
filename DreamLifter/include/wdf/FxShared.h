#pragma once

#ifndef __WDF_FX_SHARED_H__
#define __WDF_FX_SHARED_H__

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000035L)
#define STATUS_UNSUCESSFUL               ((NTSTATUS)0xC0000001L)

// Helper
extern NTSYSAPI BOOLEAN RtlEqualUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN          CaseInSensitive
);

typedef PVOID WDF_COMPONENT_GLOBALS, * PWDF_COMPONENT_GLOBALS;

DECLARE_HANDLE(WDFOBJECT);
DECLARE_HANDLE(WDFDEVICE);

DECLARE_HANDLE(WDFREQUEST);

DECLARE_HANDLE(WDFSPINLOCK);

DECLARE_HANDLE(WDFIORESREQLIST);
DECLARE_HANDLE(WDFIORESLIST);
DECLARE_HANDLE(WDFCMRESLIST);

DECLARE_HANDLE(WDFQUEUE);
DECLARE_HANDLE(WDFKEY);

DECLARE_HANDLE(WDFTIMER);
DECLARE_HANDLE(WDFWORKITEM);

typedef PVOID PDRIVER_OBJECT;
typedef PVOID PDEVICE_OBJECT;
typedef PVOID WDFCONTEXT;

typedef enum _WDF_TRI_STATE {
    WdfFalse = 0,
    WdfTrue = 1,
    WdfUseDefault = 2,
} WDF_TRI_STATE, * PWDF_TRI_STATE;

typedef
VOID
EVT_WDF_OBJECT_CONTEXT_CLEANUP(
    _In_
    WDFOBJECT Object
);

typedef EVT_WDF_OBJECT_CONTEXT_CLEANUP* PFN_WDF_OBJECT_CONTEXT_CLEANUP;

typedef
VOID
EVT_WDF_OBJECT_CONTEXT_DESTROY(
    _In_
    WDFOBJECT Object
);

typedef EVT_WDF_OBJECT_CONTEXT_DESTROY* PFN_WDF_OBJECT_CONTEXT_DESTROY;

// 
// Specifies the highest IRQL level allowed on callbacks
// to the device driver.
// 
typedef enum _WDF_EXECUTION_LEVEL {
    WdfExecutionLevelInvalid = 0x00,
    WdfExecutionLevelInheritFromParent,
    WdfExecutionLevelPassive,
    WdfExecutionLevelDispatch,
} WDF_EXECUTION_LEVEL;

// 
// Specifies the concurrency of callbacks to the device driver
// 
typedef enum _WDF_SYNCHRONIZATION_SCOPE {
    WdfSynchronizationScopeInvalid = 0x00,
    WdfSynchronizationScopeInheritFromParent,
    WdfSynchronizationScopeDevice,
    WdfSynchronizationScopeQueue,
    WdfSynchronizationScopeNone,
} WDF_SYNCHRONIZATION_SCOPE;

#define WDF_NO_OBJECT_ATTRIBUTES (NULL)
#define WDF_NO_EVENT_CALLBACK (NULL)
#define WDF_NO_HANDLE (NULL)
#define WDF_NO_CONTEXT (NULL)
#define WDF_NO_SEND_OPTIONS (NULL)

typedef const struct _WDF_OBJECT_CONTEXT_TYPE_INFO* PCWDF_OBJECT_CONTEXT_TYPE_INFO;

typedef
PCWDF_OBJECT_CONTEXT_TYPE_INFO
(__cdecl* PFN_GET_UNIQUE_CONTEXT_TYPE)(
    VOID
    );

typedef struct _WDF_OBJECT_CONTEXT_TYPE_INFO {
    ULONG                          Size;
    LPCSTR                         ContextName;
    size_t                         ContextSize;
    PCWDF_OBJECT_CONTEXT_TYPE_INFO UniqueType;
    PFN_GET_UNIQUE_CONTEXT_TYPE    EvtDriverGetUniqueContextType;
} WDF_OBJECT_CONTEXT_TYPE_INFO, * PWDF_OBJECT_CONTEXT_TYPE_INFO;

typedef struct _WDF_OBJECT_ATTRIBUTES* PWDF_OBJECT_ATTRIBUTES;
typedef struct _WDF_OBJECT_ATTRIBUTES {
    //
    // Size in bytes of this structure
    //
    ULONG Size;

    //
    // Function to call when the object is deleted
    //
    PFN_WDF_OBJECT_CONTEXT_CLEANUP EvtCleanupCallback;

    //
    // Function to call when the objects memory is destroyed when the
    // the last reference count goes to zero
    //
    PFN_WDF_OBJECT_CONTEXT_DESTROY EvtDestroyCallback;

    //
    // Execution level constraints for Object
    //
    WDF_EXECUTION_LEVEL ExecutionLevel;

    //
    // Synchronization level constraint for Object
    //
    WDF_SYNCHRONIZATION_SCOPE SynchronizationScope;

    //
    // Optional Parent Object
    //
    WDFOBJECT ParentObject;

    //
    // Overrides the size of the context allocated as specified by
    // ContextTypeInfo->ContextSize
    //
    size_t ContextSizeOverride;

    //
    // Pointer to the type information to be associated with the object
    //
    PCWDF_OBJECT_CONTEXT_TYPE_INFO ContextTypeInfo;

} WDF_OBJECT_ATTRIBUTES, * PWDF_OBJECT_ATTRIBUTES;

typedef enum _WDF_POWER_DEVICE_STATE {
    WdfPowerDeviceInvalid = 0,
    WdfPowerDeviceD0,
    WdfPowerDeviceD1,
    WdfPowerDeviceD2,
    WdfPowerDeviceD3,
    WdfPowerDeviceD3Final,
    WdfPowerDevicePrepareForHibernation,
    WdfPowerDeviceMaximum,
} WDF_POWER_DEVICE_STATE, * PWDF_POWER_DEVICE_STATE;

typedef enum _WDF_SPECIAL_FILE_TYPE {
    WdfSpecialFileUndefined = 0,
    WdfSpecialFilePaging = 1,
    WdfSpecialFileHibernation,
    WdfSpecialFileDump,
    WdfSpecialFileBoot,
    WdfSpecialFileMax,
} WDF_SPECIAL_FILE_TYPE, * PWDF_SPECIAL_FILE_TYPE;

typedef enum _DEVICE_RELATION_TYPE {
    BusRelations,
    EjectionRelations,
    PowerRelations,
    RemovalRelations,
    TargetDeviceRelation,
    SingleBusRelations,
    TransportRelations
} DEVICE_RELATION_TYPE, * PDEVICE_RELATION_TYPE;

typedef struct _DEVICE_RELATIONS {
    ULONG Count;
    PDEVICE_OBJECT Objects[1];  // variable length
} DEVICE_RELATIONS, * PDEVICE_RELATIONS;

typedef
NTSTATUS
EVT_WDF_DEVICE_D0_ENTRY(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE PreviousState
);

typedef EVT_WDF_DEVICE_D0_ENTRY* PFN_WDF_DEVICE_D0_ENTRY;

typedef
NTSTATUS
EVT_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE PreviousState
);

typedef EVT_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED* PFN_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED;

typedef
NTSTATUS
EVT_WDF_DEVICE_D0_EXIT(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE TargetState
);

typedef EVT_WDF_DEVICE_D0_EXIT* PFN_WDF_DEVICE_D0_EXIT;

typedef
NTSTATUS
EVT_WDF_DEVICE_D0_EXIT_PRE_INTERRUPTS_DISABLED(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_POWER_DEVICE_STATE TargetState
);

typedef EVT_WDF_DEVICE_D0_EXIT_PRE_INTERRUPTS_DISABLED* PFN_WDF_DEVICE_D0_EXIT_PRE_INTERRUPTS_DISABLED;

typedef
NTSTATUS
EVT_WDF_DEVICE_PREPARE_HARDWARE(
    _In_
    WDFDEVICE Device,
    _In_
    WDFCMRESLIST ResourcesRaw,
    _In_
    WDFCMRESLIST ResourcesTranslated
);

typedef EVT_WDF_DEVICE_PREPARE_HARDWARE* PFN_WDF_DEVICE_PREPARE_HARDWARE;

typedef
NTSTATUS
EVT_WDF_DEVICE_RELEASE_HARDWARE(
    _In_
    WDFDEVICE Device,
    _In_
    WDFCMRESLIST ResourcesTranslated
);

typedef EVT_WDF_DEVICE_RELEASE_HARDWARE* PFN_WDF_DEVICE_RELEASE_HARDWARE;

typedef
VOID
EVT_WDF_DEVICE_SELF_MANAGED_IO_CLEANUP(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_SELF_MANAGED_IO_CLEANUP* PFN_WDF_DEVICE_SELF_MANAGED_IO_CLEANUP;

typedef
VOID
EVT_WDF_DEVICE_SELF_MANAGED_IO_FLUSH(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_SELF_MANAGED_IO_FLUSH* PFN_WDF_DEVICE_SELF_MANAGED_IO_FLUSH;

typedef
NTSTATUS
EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT* PFN_WDF_DEVICE_SELF_MANAGED_IO_INIT;

typedef
NTSTATUS
EVT_WDF_DEVICE_SELF_MANAGED_IO_SUSPEND(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_SELF_MANAGED_IO_SUSPEND* PFN_WDF_DEVICE_SELF_MANAGED_IO_SUSPEND;

typedef
NTSTATUS
EVT_WDF_DEVICE_SELF_MANAGED_IO_RESTART(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_SELF_MANAGED_IO_RESTART* PFN_WDF_DEVICE_SELF_MANAGED_IO_RESTART;

typedef
NTSTATUS
EVT_WDF_DEVICE_QUERY_STOP(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_QUERY_STOP* PFN_WDF_DEVICE_QUERY_STOP;

typedef
NTSTATUS
EVT_WDF_DEVICE_QUERY_REMOVE(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_QUERY_REMOVE* PFN_WDF_DEVICE_QUERY_REMOVE;

typedef
VOID
EVT_WDF_DEVICE_SURPRISE_REMOVAL(
    _In_
    WDFDEVICE Device
);

typedef EVT_WDF_DEVICE_SURPRISE_REMOVAL* PFN_WDF_DEVICE_SURPRISE_REMOVAL;

typedef
VOID
EVT_WDF_DEVICE_USAGE_NOTIFICATION(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_SPECIAL_FILE_TYPE NotificationType,
    _In_
    BOOLEAN IsInNotificationPath
);

typedef EVT_WDF_DEVICE_USAGE_NOTIFICATION* PFN_WDF_DEVICE_USAGE_NOTIFICATION;

typedef
NTSTATUS
EVT_WDF_DEVICE_USAGE_NOTIFICATION_EX(
    _In_
    WDFDEVICE Device,
    _In_
    WDF_SPECIAL_FILE_TYPE NotificationType,
    _In_
    BOOLEAN IsInNotificationPath
);

typedef EVT_WDF_DEVICE_USAGE_NOTIFICATION_EX* PFN_WDF_DEVICE_USAGE_NOTIFICATION_EX;

typedef
VOID
EVT_WDF_DEVICE_RELATIONS_QUERY(
    _In_
    WDFDEVICE Device,
    _In_
    DEVICE_RELATION_TYPE RelationType
);

typedef EVT_WDF_DEVICE_RELATIONS_QUERY* PFN_WDF_DEVICE_RELATIONS_QUERY;

typedef struct _WDF_PNPPOWER_EVENT_CALLBACKS {
    ULONG                                           Size;
    PFN_WDF_DEVICE_D0_ENTRY                         EvtDeviceD0Entry;
    PFN_WDF_DEVICE_D0_ENTRY_POST_INTERRUPTS_ENABLED EvtDeviceD0EntryPostInterruptsEnabled;
    PFN_WDF_DEVICE_D0_EXIT                          EvtDeviceD0Exit;
    PFN_WDF_DEVICE_D0_EXIT_PRE_INTERRUPTS_DISABLED  EvtDeviceD0ExitPreInterruptsDisabled;
    PFN_WDF_DEVICE_PREPARE_HARDWARE                 EvtDevicePrepareHardware;
    PFN_WDF_DEVICE_RELEASE_HARDWARE                 EvtDeviceReleaseHardware;
    PFN_WDF_DEVICE_SELF_MANAGED_IO_CLEANUP          EvtDeviceSelfManagedIoCleanup;
    PFN_WDF_DEVICE_SELF_MANAGED_IO_FLUSH            EvtDeviceSelfManagedIoFlush;
    PFN_WDF_DEVICE_SELF_MANAGED_IO_INIT             EvtDeviceSelfManagedIoInit;
    PFN_WDF_DEVICE_SELF_MANAGED_IO_SUSPEND          EvtDeviceSelfManagedIoSuspend;
    PFN_WDF_DEVICE_SELF_MANAGED_IO_RESTART          EvtDeviceSelfManagedIoRestart;
    PFN_WDF_DEVICE_SURPRISE_REMOVAL                 EvtDeviceSurpriseRemoval;
    PFN_WDF_DEVICE_QUERY_REMOVE                     EvtDeviceQueryRemove;
    PFN_WDF_DEVICE_QUERY_STOP                       EvtDeviceQueryStop;
    PFN_WDF_DEVICE_USAGE_NOTIFICATION               EvtDeviceUsageNotification;
    PFN_WDF_DEVICE_RELATIONS_QUERY                  EvtDeviceRelationsQuery;
    PFN_WDF_DEVICE_USAGE_NOTIFICATION_EX            EvtDeviceUsageNotificationEx;
} WDF_PNPPOWER_EVENT_CALLBACKS, * PWDF_PNPPOWER_EVENT_CALLBACKS;

//
// WDF Function: WdfDeviceInitSetPnpPowerEventCallbacks
//
typedef
VOID
(*PFN_WDFDEVICEINITSETPNPPOWEREVENTCALLBACKS)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PVOID DeviceInit,
    _In_
    PWDF_PNPPOWER_EVENT_CALLBACKS PnpPowerEventCallbacks
    );

typedef struct WDFDEVICE_INIT* PWDFDEVICE_INIT;
typedef struct WDFCXDEVICE_INIT* PWDFCXDEVICE_INIT;

typedef
NTSTATUS
EVT_WDF_DRIVER_DEVICE_ADD(
    WDFDRIVER Driver,
    PWDFDEVICE_INIT DeviceInit
);

typedef EVT_WDF_DRIVER_DEVICE_ADD* PFN_WDF_DRIVER_DEVICE_ADD;

typedef
VOID
EVT_WDF_DRIVER_UNLOAD(
    WDFDRIVER Driver
);

typedef EVT_WDF_DRIVER_UNLOAD* PFN_WDF_DRIVER_UNLOAD;

typedef struct _WDF_DRIVER_CONFIG {
    //
    // Size of this structure in bytes
    //
    ULONG Size;

    //
    // Event callbacks
    //
    PFN_WDF_DRIVER_DEVICE_ADD EvtDriverDeviceAdd;

    PFN_WDF_DRIVER_UNLOAD    EvtDriverUnload;

    //
    // Combination of WDF_DRIVER_INIT_FLAGS values
    //
    ULONG DriverInitFlags;

    //
    // Pool tag to use for all allocations made by the framework on behalf of
    // the client driver.
    //
    ULONG DriverPoolTag;

} WDF_DRIVER_CONFIG, * PWDF_DRIVER_CONFIG;

typedef
NTSTATUS
(*PFN_WDFDRIVERCREATE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PDRIVER_OBJECT DriverObject,
    _In_
    PCUNICODE_STRING RegistryPath,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES DriverAttributes,
    _In_
    PWDF_DRIVER_CONFIG DriverConfig,
    _Out_opt_
    WDFDRIVER* Driver
    );

//
// WDF Function: WdfDeviceCreate
//
typedef
NTSTATUS
(*PFN_WDFDEVICECREATE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _Inout_
    PWDFDEVICE_INIT* DeviceInit,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES DeviceAttributes,
    _Out_
    WDFDEVICE* Device
    );

//
// WDF Function: WdfDeviceCreateDeviceInterface
//
typedef
NTSTATUS
(*PFN_WDFDEVICECREATEDEVICEINTERFACE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE Device,
    _In_
    CONST GUID* InterfaceClassGUID,
    _In_opt_
    PCUNICODE_STRING ReferenceString
    );

typedef
NTSTATUS
(*PFN_WDFSPINLOCKCREATE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES SpinLockAttributes,
    _Out_
    WDFSPINLOCK* SpinLock
    );

typedef
VOID
(*PFN_WDFSPINLOCKACQUIRE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    _Requires_lock_not_held_(_Curr_)
    _Acquires_lock_(_Curr_)
    _IRQL_saves_
    WDFSPINLOCK SpinLock
    );

typedef
VOID
(*PFN_WDFSPINLOCKRELEASE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    _Requires_lock_held_(_Curr_)
    _Releases_lock_(_Curr_)
    _IRQL_restores_
    WDFSPINLOCK SpinLock
    );

//
// WDF Function: WdfRequestComplete
//
typedef
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
(*PFN_WDFREQUESTCOMPLETE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFREQUEST Request,
    _In_
    NTSTATUS Status
    );

typedef enum _WDF_IO_QUEUE_DISPATCH_TYPE {
    WdfIoQueueDispatchInvalid = 0,
    WdfIoQueueDispatchSequential,
    WdfIoQueueDispatchParallel,
    WdfIoQueueDispatchManual,
    WdfIoQueueDispatchMax,
} WDF_IO_QUEUE_DISPATCH_TYPE;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_DEFAULT(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request
);

typedef EVT_WDF_IO_QUEUE_IO_DEFAULT* PFN_WDF_IO_QUEUE_IO_DEFAULT;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_STOP(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    ULONG ActionFlags
);

typedef EVT_WDF_IO_QUEUE_IO_STOP* PFN_WDF_IO_QUEUE_IO_STOP;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_RESUME(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request
);

typedef EVT_WDF_IO_QUEUE_IO_RESUME* PFN_WDF_IO_QUEUE_IO_RESUME;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_READ(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    size_t Length
);

typedef EVT_WDF_IO_QUEUE_IO_READ* PFN_WDF_IO_QUEUE_IO_READ;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_WRITE(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    size_t Length
);

typedef EVT_WDF_IO_QUEUE_IO_WRITE* PFN_WDF_IO_QUEUE_IO_WRITE;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    size_t OutputBufferLength,
    _In_
    size_t InputBufferLength,
    _In_
    ULONG IoControlCode
);

typedef EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL* PFN_WDF_IO_QUEUE_IO_DEVICE_CONTROL;

typedef
VOID
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request,
    _In_
    size_t OutputBufferLength,
    _In_
    size_t InputBufferLength,
    _In_
    ULONG IoControlCode
);

typedef EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL* PFN_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL;


typedef
VOID
EVT_WDF_IO_QUEUE_IO_CANCELED_ON_QUEUE(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFREQUEST Request
);

typedef EVT_WDF_IO_QUEUE_IO_CANCELED_ON_QUEUE* PFN_WDF_IO_QUEUE_IO_CANCELED_ON_QUEUE;


typedef
VOID
EVT_WDF_IO_QUEUE_STATE(
    _In_
    WDFQUEUE Queue,
    _In_
    WDFCONTEXT Context
);

typedef EVT_WDF_IO_QUEUE_STATE* PFN_WDF_IO_QUEUE_STATE;

//
// This is the structure used to configure an IoQueue and
// register callback events to it.
//
typedef struct _WDF_IO_QUEUE_CONFIG {
    ULONG                                       Size;
    WDF_IO_QUEUE_DISPATCH_TYPE                  DispatchType;
    WDF_TRI_STATE                               PowerManaged;
    BOOLEAN                                     AllowZeroLengthRequests;
    BOOLEAN                                     DefaultQueue;
    PFN_WDF_IO_QUEUE_IO_DEFAULT                 EvtIoDefault;
    PFN_WDF_IO_QUEUE_IO_READ                    EvtIoRead;
    PFN_WDF_IO_QUEUE_IO_WRITE                   EvtIoWrite;
    PFN_WDF_IO_QUEUE_IO_DEVICE_CONTROL          EvtIoDeviceControl;
    PFN_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL EvtIoInternalDeviceControl;
    PFN_WDF_IO_QUEUE_IO_STOP                    EvtIoStop;
    PFN_WDF_IO_QUEUE_IO_RESUME                  EvtIoResume;
    PFN_WDF_IO_QUEUE_IO_CANCELED_ON_QUEUE       EvtIoCanceledOnQueue;

    union {
        struct {
            ULONG NumberOfPresentedRequests;
        } Parallel;
    } Settings;

    WDFDRIVER                                   Driver;
} WDF_IO_QUEUE_CONFIG, * PWDF_IO_QUEUE_CONFIG;

//
// WDF Function: WdfIoQueueCreate
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
(*PFN_WDFIOQUEUECREATE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE Device,
    _In_
    PWDF_IO_QUEUE_CONFIG Config,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES QueueAttributes,
    _Out_opt_
    WDFQUEUE* Queue
    );

//
// WDF Function: WdfDriverOpenParametersRegistryKey
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_WDFDRIVEROPENPARAMETERSREGISTRYKEY)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDRIVER Driver,
    _In_
    ACCESS_MASK DesiredAccess,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES KeyAttributes,
    _Out_
    WDFKEY* Key
    );

//
// WDF Function: WdfRegistryClose
//
typedef
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
(*PFN_WDFREGISTRYCLOSE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key
    );

//
// WDF Function: WdfRegistryQueryULong
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
(*PFN_WDFREGISTRYQUERYULONG)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key,
    _In_
    PCUNICODE_STRING ValueName,
    _Out_
    PULONG Value
    );

#define TolerableDelayUnlimited ((ULONG)-1)

//
// This is the function that gets called back into the driver
// when the TIMER fires.
//
typedef
_Function_class_(EVT_WDF_TIMER)
_IRQL_requires_same_
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
EVT_WDF_TIMER(
    _In_
    WDFTIMER Timer
);

typedef EVT_WDF_TIMER* PFN_WDF_TIMER;

//
// Disable warning C4324: structure was padded due to DECLSPEC_ALIGN
// This padding is intentional and necessary.
#pragma warning(push)
#pragma warning(disable: 4324)

typedef struct _WDF_TIMER_CONFIG {
    ULONG Size;
    PFN_WDF_TIMER EvtTimerFunc;

    ULONG Period;

    //
    // If this is TRUE, the Timer will automatically serialize
    // with the event callback handlers of its Parent Object.
    //
    // Parent Object's callback constraints should be compatible
    // with the Timer DPC (DISPATCH_LEVEL), or the request will fail.
    //
    BOOLEAN AutomaticSerialization;

    //
    // Optional tolerance for the timer in milliseconds.
    //
    ULONG TolerableDelay;

    //
    // If this is TRUE, high resolution timers will be used. The default
    // value is FALSE
    //
    DECLSPEC_ALIGN(8) BOOLEAN UseHighResolutionTimer;

} WDF_TIMER_CONFIG, * PWDF_TIMER_CONFIG;

#pragma warning(pop)

//
// WDF Function: WdfTimerCreate
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
(*PFN_WDFTIMERCREATE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PWDF_TIMER_CONFIG Config,
    _In_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    WDFTIMER* Timer
    );

//
// WDF Function: WdfTimerGetParentObject
//
typedef
_IRQL_requires_max_(DISPATCH_LEVEL)
WDFOBJECT
(*PFN_WDFTIMERGETPARENTOBJECT)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer
    );

//
// WDF Function: WdfTimerStart
//
typedef
_IRQL_requires_max_(DISPATCH_LEVEL)
BOOLEAN
(*PFN_WDFTIMERSTART)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer,
    _In_
    LONGLONG DueTime
    );

//
// WDF Function: WdfTimerStop
//
typedef
_When_(Wait == __true, _IRQL_requires_max_(PASSIVE_LEVEL))
_When_(Wait == __false, _IRQL_requires_max_(DISPATCH_LEVEL))
BOOLEAN
(*PFN_WDFTIMERSTOP)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer,
    _In_
    BOOLEAN Wait
    );

typedef
VOID
EVT_WDF_WORKITEM(
    _In_
    WDFWORKITEM WorkItem
);

typedef EVT_WDF_WORKITEM* PFN_WDF_WORKITEM;

typedef struct _WDF_WORKITEM_CONFIG {
    ULONG            Size;
    PFN_WDF_WORKITEM EvtWorkItemFunc;
    BOOLEAN          AutomaticSerialization;
} WDF_WORKITEM_CONFIG, * PWDF_WORKITEM_CONFIG;

//
// WDF Function: WdfWorkItemCreate
//
typedef
_Must_inspect_result_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS
(*PFN_WDFWORKITEMCREATE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PWDF_WORKITEM_CONFIG Config,
    _In_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    WDFWORKITEM* WorkItem
    );

//
// WDF Function: WdfWorkItemGetParentObject
//
typedef
_IRQL_requires_max_(DISPATCH_LEVEL)
WDFOBJECT
(*PFN_WDFWORKITEMGETPARENTOBJECT)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFWORKITEM WorkItem
    );

//
// WDF Function: WdfWorkItemEnqueue
//
typedef
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID
(*PFN_WDFWORKITEMENQUEUE)(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFWORKITEM WorkItem
    );

#endif
