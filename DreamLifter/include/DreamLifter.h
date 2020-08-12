#pragma once
#ifndef _DREAMLIFTER_H_
#define _DREAMLIFTER_H_

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <winternl.h>

#include <MinUmdfLoaderInterface.h>

#include <wdf/um/UmdfDriverEnums0215.h>
#include <wdf/um/MinUmdf0215.h>

#include <wdf/um/UcmFuncEnums0100.h>

#define GUEST_DRIVER_NAME "DreamLifterGuest"

#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_NOT_SUPPORTED             ((NTSTATUS)0xC00000BBL)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002L)

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

// Begin DreamLifter WDF implementation
typedef struct _DRIVER_INSTANCE {
    int Unused; // Keep compatibility
    PFN_WDF_OBJECT_CONTEXT_CLEANUP DriverCleanupCallback;
    PFN_WDF_OBJECT_CONTEXT_DESTROY DriverDestroyCallback;
    PFN_WDF_DRIVER_DEVICE_ADD      DriverDeviceAdd;
    PFN_WDF_DRIVER_UNLOAD          DriverUnload;
} DRIVER_INSTANCE, *PDRIVER_INSTANCE;

typedef struct _DREAMLIFTER_DEVICE_INIT {
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
} DREAMLIFTER_DEVICE_INIT, *PDREAMLIFTER_DEVICE_INIT;

typedef struct _DREAMLIFTER_DEVICE {
    // This can be further extended
    PFN_WDF_DEVICE_PREPARE_HARDWARE EvtDevicePrepareHardware;
    PCWDF_OBJECT_CONTEXT_TYPE_INFO DeviceContextInfo;
    PVOID DeviceContext;
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

#endif
