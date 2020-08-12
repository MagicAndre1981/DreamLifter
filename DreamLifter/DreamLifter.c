// DreamLifter.c: load the Type-C controller library and kick start it

#include <DreamLifter.h>

PVOID g_WdfFunctions0215[WdfFunctionTableNumEntries];
PVOID g_UcmFunctions0100[UcmFunctionTableNumEntries];

WDF_LOADER_INTERFACE g_loaderInterface;
UNICODE_STRING g_FakeRegPath;
PDRIVER_INSTANCE g_pDriverInstance;
PDREAMLIFTER_DEVICE g_pDevice;

int main(int argc, char* argv[])
{
    errno_t err = 0;

    HMODULE hTycLibrary = NULL;
    PFN_WUDF_DRIVER_ENTRY_UM pTycEntry = NULL;
    NTSTATUS status;

    // What it needs to do:
    // 0. Initialize itself
    // 1. Load TyC.dll via LoadLibrary
    // 2. Initialize some structs
    // 3. Kicks in

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // Initialization
    RtlZeroMemory(g_WdfFunctions0215, sizeof(g_WdfFunctions0215));
    RtlZeroMemory(g_UcmFunctions0100, sizeof(g_UcmFunctions0100));

    // Leave all entries as not implemented
    for (ULONG i = 0; i < WdfFunctionTableNumEntries; i++) {
        g_WdfFunctions0215[i] = (PVOID) DlWdfFunctionImplStub;
    }

    for (ULONG i = 0; i < UcmFunctionTableNumEntries; i++) {
        g_UcmFunctions0100[i] = (PVOID) DlWdfFunctionImplStub;
    }

    // Now fills in the implementation
    g_WdfFunctions0215[WdfDriverCreateTableIndex] = (PVOID) DlWdfCreateDriver;
    g_WdfFunctions0215[WdfDeviceInitSetPnpPowerEventCallbacksTableIndex] = (PVOID) DlWdfDeviceInitSetPnpPowerEventCallbacks;
    g_WdfFunctions0215[WdfDeviceCreateTableIndex] = (PVOID) DlWdfDeviceCreate;
    g_WdfFunctions0215[WdfDeviceCreateDeviceInterfaceTableIndex] = (PVOID) DlWdfCreateDeviceInterface;
    g_WdfFunctions0215[WdfSpinLockCreateTableIndex] = (PVOID) DlWdfSpinLockCreate;
    g_WdfFunctions0215[WdfSpinLockAcquireTableIndex] = (PVOID) DlWdfSpinLockAcquire;
    g_WdfFunctions0215[WdfSpinLockReleaseTableIndex] = (PVOID) DlWdfSpinLockRelease;
    g_WdfFunctions0215[WdfRequestCompleteTableIndex] = (PVOID) DlWdfRequestComplete;
    g_WdfFunctions0215[WdfIoQueueCreateTableIndex] = (PVOID) DlWdfIoQueueCreate;
    g_WdfFunctions0215[WdfDriverOpenParametersRegistryKeyTableIndex] = (PVOID) DlWdfDriverOpenParametersRegistryKey;
    g_WdfFunctions0215[WdfRegistryCloseTableIndex] = (PVOID) DlWdfRegistryClose;
    g_WdfFunctions0215[WdfRegistryQueryULongTableIndex] = (PVOID) DlWdfRegistryQueryULong;
    g_WdfFunctions0215[WdfTimerCreateTableIndex] = (PVOID) DlWdfTimerCreate;

    // Prepare loader interface
    RtlZeroMemory(&g_loaderInterface, sizeof(WDF_LOADER_INTERFACE));
    g_loaderInterface.LoaderInterfaceSize = sizeof(WDF_LOADER_INTERFACE);
    g_loaderInterface.BindExtensionClass = (PFN_LOADER_BIND_EXTENSION_CLASS) DlUmBindExtensionClass;
    g_loaderInterface.BindVersionLibrary = (PFN_LOADER_BIND_VERSION_LIB) DlUmBindVersionLib;
    g_loaderInterface.LoaderFlags = 1;

    RtlInitUnicodeString(&g_FakeRegPath, L"");

    // Load TyC via LoadLibrary
    hTycLibrary = LoadLibraryA("C:\\Windows\\DreamLifter\\TyC.dll");
    if (hTycLibrary == NULL)
    {
        printf("Unload to load TyC.dll, error %d\n", GetLastError());
        err = ENFILE;
        goto exit;
    }

    // Find WUDF entry point
    pTycEntry = (PFN_WUDF_DRIVER_ENTRY_UM) GetProcAddress(hTycLibrary, "FxDriverEntryUm");
    if (pTycEntry == NULL)
    {
        printf("Failed to locate FxDriverEntryUm\n");
        err = ENOENT;
        goto exit;
    }

    // Let's kick this in
    status = pTycEntry(&g_loaderInterface, NULL, NULL, &g_FakeRegPath);
    printf("DriverEntry returns, result 0x%x\n", status);

    if (!NT_SUCCESS(status)) {
        printf("DriverEntry failed: 0x%x\n", status);
        err = RtlNtStatusToDosError(status);
        goto exit;
    }

    // Call add device event
    if (g_pDriverInstance->DriverDeviceAdd != NULL) {
        DREAMLIFTER_DEVICE_INIT deviceInit;
        RtlZeroMemory(&deviceInit, sizeof(deviceInit));
        status = g_pDriverInstance->DriverDeviceAdd((WDFDRIVER) g_pDriverInstance, (PWDFDEVICE_INIT) &deviceInit);
        if (!NT_SUCCESS(status)) {
            printf("DriverDeviceAdd failed: 0x%x\n", status);
            err = RtlNtStatusToDosError(status);
            goto cleanup;
        }
        // Start main loop here
    }

    // Cleanup
cleanup:
    if (g_pDevice != NULL) {
        if (g_pDevice->DeviceContext != NULL) {
            free(g_pDevice->DeviceContext);
        }
        if (g_pDevice->SerializationMutex != NULL) {
            // Acquire ownership and release it
            WaitForSingleObject(g_pDevice->SerializationMutex, INFINITE);
            CloseHandle(g_pDevice->SerializationMutex);
        }
    }

    if (g_pDriverInstance != NULL) {
        if (g_pDriverInstance->DriverCleanupCallback != NULL) {
            g_pDriverInstance->DriverCleanupCallback((WDFOBJECT) g_pDriverInstance); 
        }
        if (g_pDriverInstance->DriverUnload != NULL) {
            g_pDriverInstance->DriverUnload((WDFDRIVER) g_pDriverInstance);
        }

        free(g_pDriverInstance);
    }
    
exit:
    return err;
}

NTSTATUS DlUmBindVersionLib(
    _In_ PVOID Context,
    _In_ PWDF_BIND_INFO BindInfo,
    _In_ PWDF_DRIVER_GLOBALS *DriverGlobals
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
    printf("Request load UMDF version %d.%d\n", BindInfo->Version.Major, BindInfo->Version.Minor);
    if (BindInfo->Version.Major = 2 && BindInfo->Version.Minor == 15)
    {
        if (BindInfo->FuncCount == WdfFunctionTableNumEntries)
        {
            *BindInfo->FuncTable = g_WdfFunctions0215;
        }
        else
        {
            printf("WDF function table entry count mismatch\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
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
        printf("Request load WDF UcmCx extension version %d.%d\n", ClassExtensionInfo->MajorVersion, ClassExtensionInfo->MinorVersion);
        *ClassExtensionInfo->FuncTable = g_UcmFunctions0100;
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS DlWdfFunctionImplStub()
{
    // Calling a function that is not yet implemented.
    printf("Calling a unsupported function\n");

    if (IsDebuggerPresent())
    {
        DebugBreak();
    }

    return STATUS_NOT_IMPLEMENTED;
}
