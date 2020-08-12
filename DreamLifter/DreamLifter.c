// DreamLifter.c: load the Type-C controller library and kick start it

#include <DreamLifter.h>

int main(int argc, char* argv[])
{
    // What it needs to do:
    // 0. Initialize itself
    // 1. Load TyC.dll via LoadLibrary
    // 2. Find WUDF entry point
    // 3. Initialize some structs
    // 4. Kicks in

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    RtlZeroMemory(g_WdfFunctions0215, sizeof(g_WdfFunctions0215));
    RtlZeroMemory(g_UcmFunctions0100, sizeof(g_UcmFunctions0100));
    
    return 0;
}

NTSTATUS DlUmLoadVersionLib(
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
            BindInfo->FuncTable = g_WdfFunctions0215;
        }
        else
        {
            printf("WDF function table entry count mismatch\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS DlUmLoadExtensionClass(
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
    printf("Request load WDF extension %s %d.%d\n", ClassExtensionInfo->ExtensionName, ClassExtensionInfo->MajorVersion, ClassExtensionInfo->MinorVersion);
    if (strcmp("UcmCx", ClassExtensionInfo->ExtensionName))
    {
        ClassExtensionInfo->FuncTable = g_UcmFunctions0100;
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_SUPPORTED;
}
