// DreamLifter.c: load the Type-C controller library and kick start it

#include <DreamLifter.h>
#include <DreamLifterService.h>

WDF_LOADER_INTERFACE g_loaderInterface;
UNICODE_STRING g_FakeRegPath;
PDRIVER_INSTANCE g_pDriverInstance;
PDREAMLIFTER_DEVICE g_pDevice;

BOOLEAN g_bStartAsStandaloneApp = FALSE;
BOOLEAN g_bStartKmLoader = FALSE;
SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

int main(int argc, char* argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { DL_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    for (int i = 0; i < argc; i++) {
        if (strcmp(DL_STANDALONE_SWITCH, argv[i]) == 0) {
            g_bStartAsStandaloneApp = TRUE;
        }
        else if (strcmp(DL_KM_LOADER, argv[i]) == 0) {
            g_bStartKmLoader = TRUE;
        }
    }

    if (g_bStartKmLoader) {
        return DlStartKmHost();
    }

    if (!g_bStartAsStandaloneApp) {
        if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
        {
            return GetLastError();
        }
    }
    else
    {
        return DlStartUmHost();
    }

    return 0;
}

int DlStartUmHost()
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

    // Initialize libraries
    DlFxLdrGetWdfUmdf0215Functions();
    DlFxLdrGetUcmCx0100Functions();

    // Prepare loader interface
    RtlZeroMemory(&g_loaderInterface, sizeof(WDF_LOADER_INTERFACE));
    g_loaderInterface.LoaderInterfaceSize = sizeof(WDF_LOADER_INTERFACE);
    g_loaderInterface.BindExtensionClass = (PFN_LOADER_BIND_EXTENSION_CLASS)DlUmBindExtensionClass;
    g_loaderInterface.BindVersionLibrary = (PFN_LOADER_BIND_VERSION_LIB)DlUmBindVersionLib;
    g_loaderInterface.LoaderFlags = 1;

    RtlInitUnicodeString(&g_FakeRegPath, L"");

    // Load TyC via LoadLibrary
    hTycLibrary = LoadLibraryA("C:\\Windows\\DreamLifter\\TyC.dll");
    if (hTycLibrary == NULL)
    {
        // Try fallback
        hTycLibrary = LoadLibraryA("TyC.dll");
    }

    if (hTycLibrary == NULL)
    {
        printf("[ERROR] Unable to load TyC.dll, error %d\n", GetLastError());
        err = ENFILE;
        goto exit;
    }

    // Find WUDF entry point
    pTycEntry = (PFN_WUDF_DRIVER_ENTRY_UM)GetProcAddress(hTycLibrary, "FxDriverEntryUm");
    if (pTycEntry == NULL)
    {
        printf("[ERROR] Failed to locate FxDriverEntryUm\n");
        err = ENOENT;
        goto exit;
    }

    // Let's kick this in
#pragma warning(disable:6387)
    status = pTycEntry(&g_loaderInterface, NULL, NULL, &g_FakeRegPath);
#pragma warning(default:6387)
    printf("[INFO] DriverEntry returns, result 0x%x\n", status);

    if (!NT_SUCCESS(status)) {
        printf("[ERROR] DriverEntry failed: 0x%x\n", status);
        err = RtlNtStatusToDosError(status);
        goto exit;
    }

    // Call add device event
    if (g_pDriverInstance->DriverDeviceAdd != NULL) {
        DREAMLIFTER_DEVICE_INIT deviceInit;
        RtlZeroMemory(&deviceInit, sizeof(deviceInit));
        status = g_pDriverInstance->DriverDeviceAdd((WDFDRIVER)g_pDriverInstance, (PWDFDEVICE_INIT)&deviceInit);
        if (!NT_SUCCESS(status)) {
            printf("[ERROR] DriverDeviceAdd failed: 0x%x\n", status);
            err = RtlNtStatusToDosError(status);
            goto cleanup;
        }
        // Simulate PnP PrepareDevice event
        if (g_pDevice != NULL) {
            if (g_pDevice->EvtDevicePrepareHardware != NULL) {
                WDFCMRESLIST EmptyList = NULL;
#pragma warning(disable:6387)
                status = g_pDevice->EvtDevicePrepareHardware((WDFDEVICE)g_pDevice, EmptyList, EmptyList);
#pragma warning(default:6387)
                if (!NT_SUCCESS(status)) {
                    printf("[ERROR] EvtDevicePrepareHardware failed: 0x%x\n", status);
                    err = RtlNtStatusToDosError(status);
                    goto cleanup;
                }
            }

            if (g_bStartAsStandaloneApp) {
                // Start main loop and hold
                while (TRUE)
                {
                    Sleep(16);
                }
            }
            else {
                // Listen for service event
                while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
                {
                    Sleep(16);
                }
            }
        }
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
            g_pDriverInstance->DriverCleanupCallback((WDFOBJECT)g_pDriverInstance);
        }
        if (g_pDriverInstance->DriverUnload != NULL) {
            g_pDriverInstance->DriverUnload((WDFDRIVER)g_pDriverInstance);
        }

        free(g_pDriverInstance);
    }

exit:
    return err;
}

VOID WINAPI ServiceMain(
    DWORD argc, LPTSTR* argv
)
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    // Register our service control handler with the SCM
    g_StatusHandle = RegisterServiceCtrlHandler(DL_SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        goto EXIT;
    }

    // Tell the service controller we are starting
    RtlZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(L"ServiceMain: SetServiceStatus failed");
    }

    /*
     * Perform tasks necessary to start the service here
     */

     // Create a service stop event to wait on later
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        // Error creating event
        // Tell service controller we are stopped and exit
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugString(L"ServiceMain: SetServiceStatus returned error");
        }
        goto EXIT;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(L"ServiceMain: SetServiceStatus returned error");
    }

    // Start a thread that will perform the main task of the service
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    if (hThread != NULL) {
        // Wait until our worker thread exits signaling that the service needs to stop
        WaitForSingleObject(hThread, INFINITE);
    }

    /*
     * Perform any cleanup tasks
     */

    CloseHandle(g_ServiceStopEvent);

    // Tell the service controller we are stopped
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(L"ServiceMain: SetServiceStatus returned error");
    }

EXIT:
    return;
}

VOID WINAPI ServiceCtrlHandler(
    DWORD CtrlCode
)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        /*
         * Perform tasks necessary to stop the service here
         */

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugString(L"ServiceCtrlHandler: SetServiceStatus returned error");
        }

        // This will signal the worker thread to start shutting down
        SetEvent(g_ServiceStopEvent);

        break;

    default:
        break;
    }
}

DWORD WINAPI ServiceWorkerThread(
    LPVOID lpParam
)
{
    errno_t ret;

    UNREFERENCED_PARAMETER(lpParam);

    ret = DlStartUmHost();
    if (ret != 0) {
        return ERROR_CREATE_FAILED;
    }

    return ERROR_SUCCESS;
}
