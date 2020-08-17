#pragma once

#ifndef __WDF_FX_SHARED_H__
#define __WDF_FX_SHARED_H__

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000035L)
#define STATUS_UNSUCESSFUL               ((NTSTATUS)0xC0000001L)

#define CmResourceTypeConnection        132   // ResType_Connection (0x8004)

#define CM_RESOURCE_CONNECTION_CLASS_GPIO          0x01
#define CM_RESOURCE_CONNECTION_CLASS_SERIAL        0x02

#define CM_RESOURCE_CONNECTION_TYPE_GPIO_IO        0x02
#define CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C     0x01
#define CM_RESOURCE_CONNECTION_TYPE_SERIAL_SPI     0x02
#define CM_RESOURCE_CONNECTION_TYPE_SERIAL_UART    0x03

// Helper
extern NTSYSAPI BOOLEAN RtlEqualUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN          CaseInSensitive
);

typedef PVOID WDF_COMPONENT_GLOBALS, * PWDF_COMPONENT_GLOBALS;
typedef LARGE_INTEGER PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;

DECLARE_HANDLE(WDFOBJECT);
DECLARE_HANDLE(WDFDEVICE);

DECLARE_HANDLE(WDFREQUEST);

DECLARE_HANDLE(WDFSPINLOCK);
DECLARE_HANDLE(WDFWAITLOCK);

DECLARE_HANDLE(WDFIORESREQLIST);
DECLARE_HANDLE(WDFIORESLIST);
DECLARE_HANDLE(WDFCMRESLIST);

DECLARE_HANDLE(WDFQUEUE);
DECLARE_HANDLE(WDFKEY);

DECLARE_HANDLE(WDFTIMER);
DECLARE_HANDLE(WDFWORKITEM);

DECLARE_HANDLE(WDFFILEOBJECT);

DECLARE_HANDLE(WDFINTERRUPT);

DECLARE_HANDLE(WDFIOTARGET);

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

#pragma pack(push,4)
#pragma warning(disable:4201)
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

        //
        // Range of resources, inclusive.  These are physical, bus relative.
        // It is known that Port and Memory below have the exact same layout
        // as Generic.
        //

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

        //
        //

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

        //
        //

        struct {
            ULONG Level;
            ULONG Vector;
            KAFFINITY Affinity;
        } Interrupt;

        //
        // Values for message signaled interrupts are distinct in the
        // raw and translated cases.
        //

        struct {
            union {
                struct {
                    USHORT Reserved;
                    USHORT MessageCount;
                    ULONG Vector;
                    KAFFINITY Affinity;
                } Raw;

                struct {
                    ULONG Level;
                    ULONG Vector;
                    KAFFINITY Affinity;
                } Translated;
            } DUMMYUNIONNAME;
        } MessageInterrupt;

        //
        // Range of memory addresses, inclusive. These are physical, bus
        // relative. The value should be the same as the one passed to
        // HalTranslateBusAddress().
        //

        struct {
            PHYSICAL_ADDRESS Start;    // 64 bit physical addresses.
            ULONG Length;
        } Memory;

        //
        // Physical DMA channel.
        //

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

        struct {
            ULONG Channel;
            ULONG RequestLine;
            UCHAR TransferWidth;
            UCHAR Reserved1;
            UCHAR Reserved2;
            UCHAR Reserved3;
        } DmaV3;

        //
        // Device driver private data, usually used to help it figure
        // what the resource assignments decisions that were made.
        //

        struct {
            ULONG Data[3];
        } DevicePrivate;

        //
        // Bus Number information.
        //

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

        //
        // Device Specific information defined by the driver.
        // The DataSize field indicates the size of the data in bytes. The
        // data is located immediately after the DeviceSpecificData field in
        // the structure.
        //

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;

        // The following structures provide support for memory-mapped
        // IO resources greater than MAXULONG
        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length40;
        } Memory40;

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length48;
        } Memory48;

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length64;
        } Memory64;

        struct {
            UCHAR Class;
            UCHAR Type;
            UCHAR Reserved1;
            UCHAR Reserved2;
            ULONG IdLowPart;
            ULONG IdHighPart;
        } Connection;

    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, * PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#pragma warning(default:4201)
#pragma pack(pop)

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

typedef
_IRQL_requires_same_
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
EVT_WDF_DEVICE_FILE_CREATE(
    _In_
    WDFDEVICE Device,
    _In_
    WDFREQUEST Request,
    _In_
    WDFFILEOBJECT FileObject
);

typedef EVT_WDF_DEVICE_FILE_CREATE* PFN_WDF_DEVICE_FILE_CREATE;

typedef
_IRQL_requires_same_
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
EVT_WDF_FILE_CLOSE(
    _In_
    WDFFILEOBJECT FileObject
);

typedef EVT_WDF_FILE_CLOSE* PFN_WDF_FILE_CLOSE;

typedef
_IRQL_requires_same_
_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
EVT_WDF_FILE_CLEANUP(
    _In_
    WDFFILEOBJECT FileObject
);

typedef EVT_WDF_FILE_CLEANUP* PFN_WDF_FILE_CLEANUP;

typedef enum _WDF_FILEOBJECT_CLASS {
    WdfFileObjectInvalid = 0,
    WdfFileObjectNotRequired = 1,
    WdfFileObjectWdfCanUseFsContext = 2,
    WdfFileObjectWdfCanUseFsContext2 = 3,
    WdfFileObjectWdfCannotUseFsContexts = 4,
    WdfFileObjectCanBeOptional = 0x80000000,
} WDF_FILEOBJECT_CLASS, * PWDF_FILEOBJECT_CLASS;

typedef struct _WDF_FILEOBJECT_CONFIG {
    ULONG                      Size;
    PFN_WDF_DEVICE_FILE_CREATE EvtDeviceFileCreate;
    PFN_WDF_FILE_CLOSE         EvtFileClose;
    PFN_WDF_FILE_CLEANUP       EvtFileCleanup;
    WDF_TRI_STATE              AutoForwardCleanupClose;
    WDF_FILEOBJECT_CLASS       FileObjectClass;
} WDF_FILEOBJECT_CONFIG, * PWDF_FILEOBJECT_CONFIG;

// Begin DreamLifter WDF implementation
typedef enum _DREAMLIFTER_WDF_OBJECT_TYPE {
    DlObjectTypeInvalid = 0,
    DlObjectTypeDriverInstance = 1,
    DlObjectTypeDeviceInstance = 2,
    DlObjectTypeDeviceInit = 3,
    DlObjectTypeSpinLock = 4,
    DlObjectTypeTimer = 5,
    DlObjectTypeWorkItem = 6,
    DlObjectTypeIoQueue = 7,
    DlObjectTypeRequest = 8,
    DlObjectTypeMemoryBuffer = 9,
    DlObjectTypeInterrupt = 10,
    DlObjectTypeCmListTranslated = 11,
    DlObjectTypeIoTarget = 12
} DREAMLIFTER_WDF_OBJECT_TYPE, *PDREAMLIFTER_WDF_OBJECT_TYPE;

#define DREAMLIFTER_OBJECT_HEADER_MAGIC 0x544C4644

typedef struct _DREAMLIFTER_WDF_OBJECT_HEADER {
    ULONG                       Magic;
    DREAMLIFTER_WDF_OBJECT_TYPE Type;
} DREAMLIFTER_WDF_OBJECT_HEADER, *PDREAMLIFTER_WDF_OBJECT_HEADER;

typedef struct _DRIVER_INSTANCE {
    DREAMLIFTER_WDF_OBJECT_HEADER  Header;
    PFN_WDF_OBJECT_CONTEXT_CLEANUP DriverCleanupCallback;
    PFN_WDF_OBJECT_CONTEXT_DESTROY DriverDestroyCallback;
    PFN_WDF_DRIVER_DEVICE_ADD      DriverDeviceAdd;
    PFN_WDF_DRIVER_UNLOAD          DriverUnload;
} DRIVER_INSTANCE, * PDRIVER_INSTANCE;

typedef struct _DREAMLIFTER_DEVICE_INIT {
    DREAMLIFTER_WDF_OBJECT_HEADER  Header;
    // PnP power events
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
    PFN_WDF_DEVICE_RELEASE_HARDWARE EvtDeviceReleaseHardware;
    PFN_WDF_DEVICE_D0_ENTRY EvtDeviceD0Entry;
    PFN_WDF_DEVICE_D0_EXIT EvtDeviceD0Exit;
    // FileObject settings
    PFN_WDF_DEVICE_FILE_CREATE EvtDeviceFileCreate;
    PFN_WDF_FILE_CLOSE         EvtFileClose;
    PFN_WDF_FILE_CLEANUP       EvtFileCleanup;
    WDF_TRI_STATE              AutoForwardCleanupClose;
    WDF_FILEOBJECT_CLASS       FileObjectClass;
} DREAMLIFTER_DEVICE_INIT, * PDREAMLIFTER_DEVICE_INIT;

typedef struct _DREAMLIFTER_SPINLOCK {
    DREAMLIFTER_WDF_OBJECT_HEADER  Header;
    volatile long Exclusion;
} DREAMLIFTER_SPINLOCK, * PDREAMLIFTER_SPINLOCK;

NTSTATUS DlWdfSpinLockCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES SpinLockAttributes,
    _Out_
    WDFSPINLOCK* SpinLock
);

void DlWdfSpinLockAcquire(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    _Requires_lock_not_held_(_Curr_)
    _Acquires_lock_(_Curr_)
    _IRQL_saves_
    WDFSPINLOCK SpinLock
);

void DlWdfSpinLockRelease(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    _Requires_lock_held_(_Curr_)
    _Releases_lock_(_Curr_)
    _IRQL_restores_
    WDFSPINLOCK SpinLock
);

typedef struct _DREAMLIFTER_TIMER {
    DREAMLIFTER_WDF_OBJECT_HEADER  Header;
    PVOID ParentObject;
    BOOL AutomaticSerialization;
    PFN_WDF_TIMER EvtTimerFunc;
    HANDLE TimerHandle;
    volatile BOOL Cancelled;
} DREAMLIFTER_TIMER, * PDREAMLIFTER_TIMER;

NTSTATUS DlWdfTimerCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PWDF_TIMER_CONFIG Config,
    _In_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    WDFTIMER* Timer
);

WDFOBJECT DlWdfTimerGetParentObject(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer
);

BOOLEAN DlWdfTimerStart(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer,
    _In_
    LONGLONG DueTime
);

VOID CALLBACK DlTimerCallbackThreadWorker(
    _In_ PVOID   lpParameter,
    _In_ BOOLEAN TimerOrWaitFired
);

BOOLEAN DlWdfTimerStop(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFTIMER Timer,
    _In_
    BOOLEAN Wait
);

typedef struct _DREAMLIFTER_CONTEXT_HEADER {
    PCWDF_OBJECT_CONTEXT_TYPE_INFO ContextInfo;
    PVOID Context;
} DREAMLIFTER_CONTEXT_HEADER, *PDREAMLIFTER_CONTEXT_HEADER;

typedef struct _DREAMLIFTER_WORKITEM {
    DREAMLIFTER_WDF_OBJECT_HEADER Header;
    // Anything with potential context will have these two at front
    PCWDF_OBJECT_CONTEXT_TYPE_INFO WorkItemContextInfo;
    PVOID            WorkItemContext;
    // These can be further extended
    PFN_WDF_WORKITEM EvtWorkItemFunc;
    BOOLEAN          AutomaticSerialization;
    PVOID            ParentObject;
} DREAMLIFTER_WORKITEM, * PDREAMLIFTER_WORKITEM;

NTSTATUS DlWdfWorkItemCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PWDF_WORKITEM_CONFIG Config,
    _In_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    WDFWORKITEM* WorkItem
);

WDFOBJECT DlWdfWorkItemGetParentObject(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFWORKITEM WorkItem
);

VOID DlWdfWorkItemEnqueue(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFWORKITEM WorkItem
);

DWORD WINAPI DlWdfWorkItemThreadWorker(
    LPVOID lpParam
);

PVOID DlWdfObjectGetTypedContextWorker(
    PWDF_DRIVER_GLOBALS            DriverGlobals,
    WDFOBJECT                      Handle,
    PCWDF_OBJECT_CONTEXT_TYPE_INFO TypeInfo
);

NTSTATUS DlWdfCreateDriver(
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

void DlWdfDeviceInitSetFileObjectConfig(
    PWDFDEVICE_INIT        DeviceInit,
    PWDF_FILEOBJECT_CONFIG FileObjectConfig,
    PWDF_OBJECT_ATTRIBUTES FileObjectAttributes
);

NTSTATUS DlWdfDeviceCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _Inout_
    PWDFDEVICE_INIT* DeviceInit,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES DeviceAttributes,
    _Out_
    WDFDEVICE* Device
);

void DlWdfDeviceInitSetPnpPowerEventCallbacks(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PVOID DeviceInit,
    _In_
    PWDF_PNPPOWER_EVENT_CALLBACKS PnpPowerEventCallbacks
);

NTSTATUS DlWdfCreateDeviceInterface(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE Device,
    _In_
    CONST GUID* InterfaceClassGUID,
    _In_opt_
    PCUNICODE_STRING ReferenceString
);

NTSTATUS DlWdfDeviceCreateSymbolicLink(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFDEVICE        Device,
    PCUNICODE_STRING SymbolicLinkName
);

typedef
BOOLEAN
EVT_WDF_INTERRUPT_ISR(
    _In_
    WDFINTERRUPT Interrupt,
    _In_
    ULONG MessageID
);

typedef EVT_WDF_INTERRUPT_ISR* PFN_WDF_INTERRUPT_ISR;

typedef
NTSTATUS
EVT_WDF_INTERRUPT_ENABLE(
    _In_
    WDFINTERRUPT Interrupt,
    _In_
    WDFDEVICE AssociatedDevice
);

typedef EVT_WDF_INTERRUPT_ENABLE* PFN_WDF_INTERRUPT_ENABLE;

typedef
NTSTATUS
EVT_WDF_INTERRUPT_DISABLE(
    _In_
    WDFINTERRUPT Interrupt,
    _In_
    WDFDEVICE AssociatedDevice
);

typedef EVT_WDF_INTERRUPT_DISABLE* PFN_WDF_INTERRUPT_DISABLE;

typedef
VOID
EVT_WDF_INTERRUPT_DPC(
    _In_
    WDFINTERRUPT Interrupt,
    _In_
    WDFOBJECT AssociatedObject
);

typedef EVT_WDF_INTERRUPT_DPC* PFN_WDF_INTERRUPT_DPC;

typedef
VOID
EVT_WDF_INTERRUPT_WORKITEM(
    _In_
    WDFINTERRUPT Interrupt,
    _In_
    WDFOBJECT AssociatedObject
);

typedef EVT_WDF_INTERRUPT_WORKITEM* PFN_WDF_INTERRUPT_WORKITEM;

typedef struct _DL_WDF_INTERRUPT {
    DREAMLIFTER_WDF_OBJECT_HEADER Header;
    WDFDEVICE AssociatedDevice;
    PFN_WDF_INTERRUPT_ISR EvtInterruptIsr;
    PFN_WDF_INTERRUPT_ENABLE EvtInterruptEnable;
    PFN_WDF_INTERRUPT_DISABLE EvtInterruptDisable;
} DL_WDF_INTERRUPT, *PDL_WDF_INTERRUPT;

typedef struct _WDF_INTERRUPT_CONFIG {
    ULONG                           Size;
    WDFSPINLOCK                     SpinLock;
    WDF_TRI_STATE                   ShareVector;
    BOOLEAN                         FloatingSave;
    BOOLEAN                         AutomaticSerialization;
    PFN_WDF_INTERRUPT_ISR           EvtInterruptIsr;
    PFN_WDF_INTERRUPT_DPC           EvtInterruptDpc;
    PFN_WDF_INTERRUPT_ENABLE        EvtInterruptEnable;
    PFN_WDF_INTERRUPT_DISABLE       EvtInterruptDisable;
    PFN_WDF_INTERRUPT_WORKITEM      EvtInterruptWorkItem;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR InterruptRaw;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR InterruptTranslated;
    WDFWAITLOCK                     WaitLock;
    BOOLEAN                         PassiveHandling;
    WDF_TRI_STATE                   ReportInactiveOnPowerDown;
    BOOLEAN                         CanWakeDevice;
} WDF_INTERRUPT_CONFIG, * PWDF_INTERRUPT_CONFIG;

NTSTATUS DlWdfInterruptCreate(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFDEVICE              Device,
    PWDF_INTERRUPT_CONFIG  Configuration,
    PWDF_OBJECT_ATTRIBUTES Attributes,
    WDFINTERRUPT* Interrupt
);

void DlWdfInterruptEnable(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFINTERRUPT Interrupt
);

void DlWdfInterruptDisable(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFINTERRUPT Interrupt
);

typedef struct _DL_WDF_QUEUE {
    DREAMLIFTER_WDF_OBJECT_HEADER Header;
    WDFDEVICE AssociatedDevice;
    PFN_WDF_IO_QUEUE_IO_DEVICE_CONTROL          EvtIoDeviceControl;
    PFN_WDF_IO_QUEUE_IO_CANCELED_ON_QUEUE       EvtIoCanceledOnQueue;
} DL_WDF_QUEUE, *PDL_WDF_QUEUE;

NTSTATUS DlWdfIoQueueCreate(
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

typedef struct _DL_WDF_CM_RES_LIST {
    DREAMLIFTER_WDF_OBJECT_HEADER Header;
    ULONG ResourceCount;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Resources;
} DL_WDF_CM_RES_LIST, *PDL_WDF_CM_RES_LIST;

ULONG DlWdfCmResourceListGetCount(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFCMRESLIST List
);

PCM_PARTIAL_RESOURCE_DESCRIPTOR DlWdfCmResourceListGetDescriptor(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFCMRESLIST List,
    ULONG        Index
);

typedef struct _DL_WDF_IOTARGET {
    DREAMLIFTER_WDF_OBJECT_HEADER Header;
    UNICODE_STRING TargetDeviceName;
    PVOID ParentDevice;
    ULONG TargetInternalId;
    BOOLEAN Opened;
} DL_WDF_IOTARGET, *PDL_WDF_IOTARGET;

typedef enum _WDF_IO_TARGET_OPEN_TYPE {
    WdfIoTargetOpenUndefined,
    WdfIoTargetOpenUseExistingDevice,
    WdfIoTargetOpenByName,
    WdfIoTargetOpenReopen,
    WdfIoTargetOpenLocalTargetByFile
} WDF_IO_TARGET_OPEN_TYPE;

typedef
NTSTATUS
EVT_WDF_IO_TARGET_QUERY_REMOVE(
    _In_
    WDFIOTARGET IoTarget
);

typedef EVT_WDF_IO_TARGET_QUERY_REMOVE* PFN_WDF_IO_TARGET_QUERY_REMOVE;

typedef
VOID
EVT_WDF_IO_TARGET_REMOVE_CANCELED(
    _In_
    WDFIOTARGET IoTarget
);

typedef EVT_WDF_IO_TARGET_REMOVE_CANCELED* PFN_WDF_IO_TARGET_REMOVE_CANCELED;

typedef
VOID
EVT_WDF_IO_TARGET_REMOVE_COMPLETE(
    _In_
    WDFIOTARGET IoTarget
);

typedef EVT_WDF_IO_TARGET_REMOVE_COMPLETE* PFN_WDF_IO_TARGET_REMOVE_COMPLETE;

typedef struct _WDF_IO_TARGET_OPEN_PARAMS {
    ULONG                             Size;
    WDF_IO_TARGET_OPEN_TYPE           Type;
    PFN_WDF_IO_TARGET_QUERY_REMOVE    EvtIoTargetQueryRemove;
    PFN_WDF_IO_TARGET_REMOVE_CANCELED EvtIoTargetRemoveCanceled;
    PFN_WDF_IO_TARGET_REMOVE_COMPLETE EvtIoTargetRemoveComplete;
    PDEVICE_OBJECT                    TargetDeviceObject;
    PVOID                             TargetFileObject;
    UNICODE_STRING                    TargetDeviceName;
    ACCESS_MASK                       DesiredAccess;
    ULONG                             ShareAccess;
    ULONG                             FileAttributes;
    ULONG                             CreateDisposition;
    ULONG                             CreateOptions;
    PVOID                             EaBuffer;
    ULONG                             EaBufferLength;
    PLONGLONG                         AllocationSize;
    ULONG                             FileInformation;
    UNICODE_STRING                    FileName;
} WDF_IO_TARGET_OPEN_PARAMS, * PWDF_IO_TARGET_OPEN_PARAMS;

NTSTATUS DlWdfIoTargetCreate(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFDEVICE Device,
    PWDF_OBJECT_ATTRIBUTES IoTargetAttributes,
    WDFIOTARGET* IoTarget
);

void DlWdfObjectDelete(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFOBJECT Object
);

NTSTATUS DlWdfIoTargetOpen(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFIOTARGET                IoTarget,
    PWDF_IO_TARGET_OPEN_PARAMS OpenParams
);

void DlWdfIoTargetClose(
    PWDF_DRIVER_GLOBALS DriverGlobals,
    WDFIOTARGET IoTarget
);

#endif
