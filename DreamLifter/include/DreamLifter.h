#pragma once

#ifndef _DREAMLIFTER_H_
#define _DREAMLIFTER_H_

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <winternl.h>
#include <wintrust.h>

#include <MinUmdfLoaderInterface.h>

#include <wdf/um/UmdfDriverEnums0215.h>
#include <wdf/um/MinUmdf0215.h>

#include <wdf/UcmCx/UcmFuncEnums0100.h>
#include <wdf/UcmCx/MinUcmCx0100.h>

#define GUEST_DRIVER_NAME "DreamLifterGuest"
#define DL_KM_LOADER "--km"

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000035L)
#define STATUS_UNSUCESSFUL               ((NTSTATUS)0xC0000001L)

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

NTSTATUS DlWdfFunctionImplStub();
NTSTATUS DlWdfCxUcmFunctionImplStub();

// Helper
extern NTSYSAPI BOOLEAN RtlEqualUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN          CaseInSensitive
);

// Begin DreamLifter WDF implementation
typedef struct _DRIVER_INSTANCE {
    PFN_WDF_OBJECT_CONTEXT_CLEANUP DriverCleanupCallback;
    PFN_WDF_OBJECT_CONTEXT_DESTROY DriverDestroyCallback;
    PFN_WDF_DRIVER_DEVICE_ADD      DriverDeviceAdd;
    PFN_WDF_DRIVER_UNLOAD          DriverUnload;
} DRIVER_INSTANCE, *PDRIVER_INSTANCE;

typedef struct _DREAMLIFTER_DEVICE_INIT {
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
} DREAMLIFTER_DEVICE_INIT, *PDREAMLIFTER_DEVICE_INIT;

typedef struct _DREAMLIFTER_UCM_MANAGER {
    HANDLE ProxyDriverHandle;
    ULONGLONG ConnectorId;
    BOOL Connected;

    UCM_CONNECTOR_TYPEC_CONFIG TypeCConfig;
    UCM_CONNECTOR_PD_CONFIG PdConfig;

    UCM_CHARGING_STATE ChargingState;
    UCM_PD_CONN_STATE PdConnState;
    UCM_TYPEC_CURRENT PowerCurrent;

    UCM_TYPEC_PARTNER Partner;
    UCM_DATA_ROLE DataRole;
    UCM_POWER_ROLE PowerRole;

    UCM_PD_REQUEST_DATA_OBJECT PdRdo;
    UCM_PD_POWER_DATA_OBJECT SourcePdos[256];
    UCHAR SourcePdoCount;
    UCM_PD_POWER_DATA_OBJECT PartnerPdos[256];
    UCHAR PartnerPdoCount;
} DREAMLIFTER_UCM_DEVICE, *PDREAMLIFTER_UCM_DEVICE;

typedef struct _DREAMLIFTER_DEVICE {
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
    PCWDF_OBJECT_CONTEXT_TYPE_INFO DeviceContextInfo;
    PVOID DeviceContext;
    HANDLE SerializationMutex;
    PDREAMLIFTER_UCM_DEVICE UcmManagerInfo;
} DREAMLIFTER_DEVICE, *PDREAMLIFTER_DEVICE;

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
} DREAMLIFTER_SPINLOCK, *PDREAMLIFTER_SPINLOCK;

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
} DREAMLIFTER_TIMER, *PDREAMLIFTER_TIMER;

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

} DREAMLIFTER_WORKITEM, *PDREAMLIFTER_WORKITEM;

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

NTSTATUS DlUcmInitializeDevice(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_MANAGER_CONFIG Config
);

NTSTATUS DlUcmCreateConnector(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_CONNECTOR_CONFIG Config,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    UCMCONNECTOR* Connector
);

NTSTATUS DlUcmConnectorTypeCAttach(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params
);

NTSTATUS DlUcmConnectorTypeCDetach(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector
);

NTSTATUS DlUcmConnectorTypeCCurrentAdChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_TYPEC_CURRENT CurrentAdvertisement
);

NTSTATUS DlUcmConnectorPdSourceCaps(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
);

NTSTATUS DlUcmConnectorPdPartnerSourceCaps(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
);

NTSTATUS DlUcmConnectorPdConnectionStateChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params
);

NTSTATUS DlUcmConnectorChargingStateChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_CHARGING_STATE ChargingState
);

NTSTATUS DlUcmConnectorDataDirectionChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_DATA_ROLE CurrentDataRole
);

NTSTATUS DlUcmConnectorPowerDirectionChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_POWER_ROLE CurrentPowerRole
);

// Debug utilities
char* DbgUcmGetOperatingMode(UCM_TYPEC_OPERATING_MODE mode);
char* DbgUcmGetPartner(UCM_TYPEC_PARTNER partner);
char* DbgUcmGetCurrent(UCM_TYPEC_CURRENT current);
char* DbgUcmGetPowerRole(UCM_POWER_ROLE role);
char* DbgUcmGetDataRole(UCM_DATA_ROLE role);
char* DbgUcmGetPdConnState(UCM_PD_CONN_STATE state);
char* DbgUcmGetChargingState(UCM_CHARGING_STATE state);
char* DbgUcmGetPdoType(UCM_PD_POWER_DATA_OBJECT_TYPE type);
void DbgUcmDumpPdo(UCM_PD_POWER_DATA_OBJECT Pdo);

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define FILE_DEVICE_CONTROLLER          0x00000004

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Proxy device IOCTLs and structs
// Starting from here, platform bitness is also needed to be taken care of
//

#define FILE_ANY_ACCESS                 0
#define FILE_SPECIAL_ACCESS    (FILE_ANY_ACCESS)
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe


// UcmConnectorTypeCAttach
#define IOCTL_UCMPROXY_TYPEC_ATTACH CTL_CODE(FILE_DEVICE_CONTROLLER, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorTypeCDetach
#define IOCTL_UCMPROXY_TYPEC_DETACH CTL_CODE(FILE_DEVICE_CONTROLLER, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorTypeCCurrentAdChanged
#define IOCTL_UCMPROXY_TYPEC_CURRENT_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPdSourceCaps
#define IOCTL_UCMPROXY_TYPEC_PD_SOURCE_CAPS CTL_CODE(FILE_DEVICE_CONTROLLER, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPdPartnerSourceCaps
#define IOCTL_UCMPROXY_TYPEC_PD_PARTNER_CAPS CTL_CODE(FILE_DEVICE_CONTROLLER, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPdConnectionStateChanged
#define IOCTL_UCMPROXY_TYPEC_PD_CONN_STATE_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorChargingStateChanged
#define IOCTL_UCMPROXY_TYPEC_CHARGING_STATE_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorDataDirectionChanged
#define IOCTL_UCMPROXY_TYPEC_DATA_DIRECTION_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x907, METHOD_BUFFERED, FILE_ANY_ACCESS)

// UcmConnectorPowerDirectionChanged
#define IOCTL_UCMPROXY_TYPEC_POWER_DIRECTION_CHANGE CTL_CODE(FILE_DEVICE_CONTROLLER, 0x908, METHOD_BUFFERED, FILE_ANY_ACCESS)

// EVT_UCM_CONNECTOR_SET_DATA_ROLE
#define IOCTL_UCMPROXY_WAIT_SET_DATA_ROLE_CALLBACK CTL_CODE(FILE_DEVICE_CONTROLLER, 0x909, METHOD_BUFFERED, FILE_ANY_ACCESS)

// EVT_UCM_CONNECTOR_SET_POWER_ROLE
#define IOCTL_UCMPROXY_WAIT_SET_POWER_ROLE_CALLBACK CTL_CODE(FILE_DEVICE_CONTROLLER, 0x910, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _PROXY_POWER_CAPS_EXCHANGE_REQUEST {
    ULONG Size;
    UCHAR PdoCount;
    UCM_PD_POWER_DATA_OBJECT Pdos[256];
} PROXY_POWER_CAPS_EXCHANGE_REQUEST, * PPROXY_POWER_CAPS_EXCHANGE_REQUEST;

typedef struct _PROXY_ROLE_CHANGE_REQUEST {
    BOOLEAN Success;
    UCHAR   Role;
} PROXY_ROLE_CHANGE_REQUEST, * PPROXY_ROLE_CHANGE_REQUEST;

DWORD WINAPI DlUcmPowerRoleEventWorker(
    LPVOID lpParam
);

DWORD WINAPI DlUcmDataRoleEventWorker(
    LPVOID lpParam
);

int DlStartDriverHost();

void DlKmLoaderStart();
void DlKmImplementationStub();

inline void CpuDeadLoop()
{
    while (TRUE) { ; }
}

#endif
