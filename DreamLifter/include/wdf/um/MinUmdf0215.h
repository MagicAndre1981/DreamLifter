#pragma once

#ifndef __MIN_UMDF_0215_H__
#define __MIN_UMDF_0215_H__

// Leave any future UMDF specific bits here
PVOID DlFxLdrGetWdfUmdf0215Functions();

NTSTATUS DlUmBindVersionLib(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS* DriverGlobals
);

NTSTATUS DlUmBindExtensionClass(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_ PWDF_CLASS_EXTENSION ClassExtensionInfo
);

NTSTATUS DlWdfUmFunctionImplStub();

// Begin DreamLifter WDF implementation
typedef struct _DRIVER_INSTANCE {
    PFN_WDF_OBJECT_CONTEXT_CLEANUP DriverCleanupCallback;
    PFN_WDF_OBJECT_CONTEXT_DESTROY DriverDestroyCallback;
    PFN_WDF_DRIVER_DEVICE_ADD      DriverDeviceAdd;
    PFN_WDF_DRIVER_UNLOAD          DriverUnload;
} DRIVER_INSTANCE, * PDRIVER_INSTANCE;

typedef struct _DREAMLIFTER_DEVICE_INIT {
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
} DREAMLIFTER_DEVICE_INIT, * PDREAMLIFTER_DEVICE_INIT;

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

void DlWdfDeviceInitSetPnpPowerEventCallbacks(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PVOID DeviceInit,
    _In_
    PWDF_PNPPOWER_EVENT_CALLBACKS PnpPowerEventCallbacks
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

typedef struct _DREAMLIFTER_SPINLOCK {
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

void DlWdfRequestComplete(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFREQUEST Request,
    _In_
    NTSTATUS Status
);

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

NTSTATUS DlWdfDriverOpenParametersRegistryKey(
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

VOID DlWdfRegistryClose(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key
);

NTSTATUS DlWdfRegistryQueryULong(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key,
    _In_
    PCUNICODE_STRING ValueName,
    _Out_
    PULONG Value
);

typedef struct _DREAMLIFTER_TIMER {
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

typedef struct _DREAMLIFTER_WORKITEM {
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

#endif
