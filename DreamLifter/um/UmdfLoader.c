// UmdfLoader.c: provides WDF UMDF 2.15 binding to the driver host

#include <DreamLifter.h>
#include <wdf/um/UmdfDriverEnums0215.h>

PVOID g_WdfFunctions0215[WdfFunctionTableNumEntries];

PVOID DlFxLdrGetWdfUmdf0215Functions()
{
    RtlZeroMemory(g_WdfFunctions0215, sizeof(g_WdfFunctions0215));

    for (ULONG i = 0; i < WdfFunctionTableNumEntries; i++) {
        g_WdfFunctions0215[i] = (PVOID)DlWdfUmFunctionImplStub;
    }

    g_WdfFunctions0215[WdfDriverCreateTableIndex] = (PVOID)DlWdfCreateDriver;
    g_WdfFunctions0215[WdfDeviceInitSetPnpPowerEventCallbacksTableIndex] = (PVOID)DlWdfDeviceInitSetPnpPowerEventCallbacks;
    g_WdfFunctions0215[WdfDeviceCreateTableIndex] = (PVOID)DlWdfDeviceCreate;
    g_WdfFunctions0215[WdfDeviceCreateDeviceInterfaceTableIndex] = (PVOID)DlWdfCreateDeviceInterface;
    g_WdfFunctions0215[WdfSpinLockCreateTableIndex] = (PVOID)DlWdfSpinLockCreate;
    g_WdfFunctions0215[WdfSpinLockAcquireTableIndex] = (PVOID)DlWdfSpinLockAcquire;
    g_WdfFunctions0215[WdfSpinLockReleaseTableIndex] = (PVOID)DlWdfSpinLockRelease;
    g_WdfFunctions0215[WdfRequestCompleteTableIndex] = (PVOID)DlWdfRequestComplete;
    g_WdfFunctions0215[WdfIoQueueCreateTableIndex] = (PVOID)DlWdfIoQueueCreate;
    g_WdfFunctions0215[WdfDriverOpenParametersRegistryKeyTableIndex] = (PVOID)DlWdfDriverOpenParametersRegistryKey;
    g_WdfFunctions0215[WdfRegistryCloseTableIndex] = (PVOID)DlWdfRegistryClose;
    g_WdfFunctions0215[WdfRegistryQueryULongTableIndex] = (PVOID)DlWdfRegistryQueryULong;
    g_WdfFunctions0215[WdfTimerCreateTableIndex] = (PVOID)DlWdfTimerCreate;
    g_WdfFunctions0215[WdfTimerGetParentObjectTableIndex] = (PVOID)DlWdfTimerGetParentObject;
    g_WdfFunctions0215[WdfTimerStartTableIndex] = (PVOID)DlWdfTimerStart;
    g_WdfFunctions0215[WdfTimerStopTableIndex] = (PVOID)DlWdfTimerStop;
    g_WdfFunctions0215[WdfWorkItemCreateTableIndex] = (PVOID)DlWdfWorkItemCreate;
    g_WdfFunctions0215[WdfWorkItemGetParentObjectTableIndex] = (PVOID)DlWdfWorkItemGetParentObject;
    g_WdfFunctions0215[WdfWorkItemEnqueueTableIndex] = (PVOID)DlWdfWorkItemEnqueue;

    return (PVOID) g_WdfFunctions0215;
}

NTSTATUS DlUmBindVersionLib(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS* DriverGlobals
)
{
    UNREFERENCED_PARAMETER(Context);

    if (BindInfo == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    *DriverGlobals = malloc(sizeof(WDF_DRIVER_GLOBALS));
    if (*DriverGlobals == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Set some fields. Can't set all yet
    RtlCopyMemory((*DriverGlobals)->DriverName, GUEST_DRIVER_NAME, sizeof(GUEST_DRIVER_NAME));
    (*DriverGlobals)->DisplaceDriverUnload = FALSE;

    // Check FX version
    // Bind to the stub WDF implementation
    printf("[INFO] Request load UMDF version %d.%d\n", BindInfo->Version.Major, BindInfo->Version.Minor);
    if (BindInfo->Version.Major = 2 && BindInfo->Version.Minor == 15)
    {
        if (BindInfo->FuncCount == WdfFunctionTableNumEntries)
        {
            *BindInfo->FuncTable = g_WdfFunctions0215;
        }
        else
        {
            printf("[INFO] WDF function table entry count mismatch\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
}
