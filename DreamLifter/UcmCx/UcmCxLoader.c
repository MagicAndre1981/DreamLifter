// UcmCxLoader.c: provides WDF UCM CX 1.00 binding to the driver host

#include <DreamLifter.h>
#include <wdf/UcmCx/UcmFuncEnums0100.h>

PVOID g_UcmFunctions0100[UcmFunctionTableNumEntries];

PVOID DlFxLdrGetUcmCx0100Functions()
{
	RtlZeroMemory(g_UcmFunctions0100, sizeof(g_UcmFunctions0100));
    for (ULONG i = 0; i < UcmFunctionTableNumEntries; i++) {
        g_UcmFunctions0100[i] = (PVOID)DlWdfCxUcmFunctionImplStub;
    }

    g_UcmFunctions0100[UcmInitializeDeviceTableIndex] = (PVOID)DlUcmInitializeDevice;
    g_UcmFunctions0100[UcmConnectorCreateTableIndex] = (PVOID)DlUcmCreateConnector;
    g_UcmFunctions0100[UcmConnectorTypeCAttachTableIndex] = (PVOID)DlUcmConnectorTypeCAttach;
    g_UcmFunctions0100[UcmConnectorTypeCDetachTableIndex] = (PVOID)DlUcmConnectorTypeCDetach;
    g_UcmFunctions0100[UcmConnectorTypeCCurrentAdChangedTableIndex] = (PVOID)DlUcmConnectorTypeCCurrentAdChanged;
    g_UcmFunctions0100[UcmConnectorPdSourceCapsTableIndex] = (PVOID)DlUcmConnectorPdSourceCaps;
    g_UcmFunctions0100[UcmConnectorPdPartnerSourceCapsTableIndex] = (PVOID)DlUcmConnectorPdPartnerSourceCaps;
    g_UcmFunctions0100[UcmConnectorPdConnectionStateChangedTableIndex] = (PVOID)DlUcmConnectorPdConnectionStateChanged;
    g_UcmFunctions0100[UcmConnectorChargingStateChangedTableIndex] = (PVOID)DlUcmConnectorChargingStateChanged;
    g_UcmFunctions0100[UcmConnectorDataDirectionChangedTableIndex] = (PVOID)DlUcmConnectorDataDirectionChanged;
    g_UcmFunctions0100[UcmConnectorPowerDirectionChangedTableIndex] = (PVOID)DlUcmConnectorPowerDirectionChanged;

    return g_UcmFunctions0100;
}

NTSTATUS DlUmBindExtensionClass(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_ PWDF_CLASS_EXTENSION ClassExtensionInfo
)
{
    UNREFERENCED_PARAMETER(Context);

    if (BindInfo == NULL || DriverGlobals == NULL || ClassExtensionInfo == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    // Bind to the stub class extension implementation
    if (strcmp("UcmCx", ClassExtensionInfo->ExtensionName))
    {
        if (ClassExtensionInfo->FuncCount == 11) {
            printf("[INFO] Request load WDF UcmCx extension version %d.%d\n", ClassExtensionInfo->MajorVersion, ClassExtensionInfo->MinorVersion);
            RtlCopyMemory((PVOID)ClassExtensionInfo->FuncTable, g_UcmFunctions0100, sizeof(g_UcmFunctions0100));
            return STATUS_SUCCESS;
        }
        else {
            printf("[INFO] Request load WDF UcmCx extension version %d.%d, but function count mismatch: %d (expect 11)\n", ClassExtensionInfo->MajorVersion,
                ClassExtensionInfo->MinorVersion, ClassExtensionInfo->FuncCount);
        }

    }

    return STATUS_NOT_SUPPORTED;
}
