// DreamLifter.c: load the Type-C controller library and kick start it

#include <DreamLifter.h>

PVOID g_WdfFunctions0215[WdfFunctionTableNumEntries];
PVOID g_UcmFunctions0100[UcmFunctionTableNumEntries];

WDF_LOADER_INTERFACE g_loaderInterface;
UNICODE_STRING g_FakeRegPath;

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

    // Prepare loader interface
    RtlZeroMemory(&g_loaderInterface, sizeof(WDF_LOADER_INTERFACE));
    g_loaderInterface.LoaderInterfaceSize = sizeof(WDF_LOADER_INTERFACE);
    g_loaderInterface.BindExtensionClass = (PFN_LOADER_BIND_EXTENSION_CLASS) DlUmBindExtensionClass;
    g_loaderInterface.BindVersionLibrary = (PFN_LOADER_BIND_VERSION_LIB) DlUmBindVersionLib;
    g_loaderInterface.LoaderFlags = 1;

    RtlInitUnicodeString(&g_FakeRegPath, L"");

    // Load TyC via LoadLibrary
    hTycLibrary = LoadLibraryW(L"TyC.dll");
    if (hTycLibrary == NULL)
    {
        printf("Unload to load TyC.dll\n");
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
    printf("Request load WDF extension %s %d.%d\n", ClassExtensionInfo->ExtensionName, ClassExtensionInfo->MajorVersion, ClassExtensionInfo->MinorVersion);
    if (strcmp("UcmCx", ClassExtensionInfo->ExtensionName))
    {
        ClassExtensionInfo->FuncTable = g_UcmFunctions0100;
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
