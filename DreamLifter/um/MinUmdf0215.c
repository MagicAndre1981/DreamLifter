// MinUmdf0215.c: UMDF 2.15 min required implementation

#include <DreamLifter.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;

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
)
{
    PDRIVER_INSTANCE pDriverInstance = NULL;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    if (DriverConfig == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // Access DriverConfig to get callbacks such as device add
    UNREFERENCED_PARAMETER(DriverConfig);

    // Initialize driver struct
    pDriverInstance = malloc(sizeof(DRIVER_INSTANCE));
    RtlZeroMemory(pDriverInstance, sizeof(DRIVER_INSTANCE));
    if (pDriverInstance == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate DRIVER_INSTANCE struct\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Driver = (WDFDRIVER*) pDriverInstance;
    // Currently this is a singleton host
    g_pDriverInstance = pDriverInstance;

    if (DriverAttributes != NULL) {
        pDriverInstance->DriverCleanupCallback = DriverAttributes->EvtCleanupCallback;
        pDriverInstance->DriverDestroyCallback = DriverAttributes->EvtCleanupCallback;
    }
    
    // This will be called when entry successfully exits
    pDriverInstance->DriverDeviceAdd = DriverConfig->EvtDriverDeviceAdd;
    pDriverInstance->DriverUnload = DriverConfig->EvtDriverUnload;

    return STATUS_SUCCESS;
}

void DlWdfDeviceInitSetPnpPowerEventCallbacks(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    PVOID DeviceInit,
    _In_
    PWDF_PNPPOWER_EVENT_CALLBACKS PnpPowerEventCallbacks
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);

    if (DeviceInit != NULL && PnpPowerEventCallbacks != NULL) {
        ((PDREAMLIFTER_DEVICE_INIT)DeviceInit)->EvtDevicePrepareHardware = PnpPowerEventCallbacks->EvtDevicePrepareHardware;
    }
}

NTSTATUS DlWdfDeviceCreate(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _Inout_
    PWDFDEVICE_INIT* DeviceInit,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES DeviceAttributes,
    _Out_
    WDFDEVICE* Device
)
{
    PDREAMLIFTER_DEVICE pDevice;
    size_t contextSize = 0;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (DeviceInit == NULL || DeviceAttributes == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    pDevice = malloc(sizeof(DREAMLIFTER_DEVICE));
    if (pDevice == NULL) {
        OutputDebugString(L"[ERROR] Failed to allocate DREAMLIFTER_DEVICE struct\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pDevice, sizeof(DREAMLIFTER_DEVICE));
    pDevice->EvtDevicePrepareHardware = ((PDREAMLIFTER_DEVICE_INIT) DeviceInit)->EvtDevicePrepareHardware;
    if (DeviceAttributes->ContextTypeInfo != NULL) {
        pDevice->DeviceContextInfo = DeviceAttributes->ContextTypeInfo;
        contextSize = (DeviceAttributes->ContextSizeOverride > DeviceAttributes->ContextTypeInfo->ContextSize) ? 
            DeviceAttributes->ContextSizeOverride : DeviceAttributes->ContextTypeInfo->ContextSize;
        if (contextSize > 0) {
            pDevice->DeviceContext = malloc(contextSize);
            if (pDevice->DeviceContext == NULL) {
                OutputDebugString(L"[ERROR] Failed to allocate device context struct\n");
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            RtlZeroMemory(pDevice->DeviceContext, contextSize);
        }
        else {
            OutputDebugString(L"[WARN] Unexpected device context size 0\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

    Device = (WDFDEVICE*) pDevice;
    g_pDevice = pDevice;
    return STATUS_SUCCESS;
}
