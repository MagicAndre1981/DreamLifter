#pragma once

#ifndef __MIN_UMDF_0215_H__
#define __MIN_UMDF_0215_H__

DECLARE_HANDLE(WDFOBJECT);
DECLARE_HANDLE(WDFDEVICE);

DECLARE_HANDLE(WDFREQUEST);

DECLARE_HANDLE(WDFSPINLOCK);

DECLARE_HANDLE(WDFIORESREQLIST);
DECLARE_HANDLE(WDFIORESLIST);
DECLARE_HANDLE(WDFCMRESLIST);

typedef PVOID PDRIVER_OBJECT;
typedef PVOID PDEVICE_OBJECT;

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

} WDF_DRIVER_CONFIG, *PWDF_DRIVER_CONFIG;

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

#endif