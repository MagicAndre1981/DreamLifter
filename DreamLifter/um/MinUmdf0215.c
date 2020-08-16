// MinUmdf0215.c: UMDF 2.15 min required implementation

#include <DreamLifter.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;

NTSTATUS DlWdfCreateDeviceInterface(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE Device,
    _In_
    CONST GUID* InterfaceClassGUID,
    _In_opt_
    PCUNICODE_STRING ReferenceString
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(ReferenceString);

    if (Device == NULL || InterfaceClassGUID == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    printf("[INFO] Creating device interface {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}. This is currently a no-op\n",
        InterfaceClassGUID->Data1, InterfaceClassGUID->Data2, InterfaceClassGUID->Data3,
        InterfaceClassGUID->Data4[0], InterfaceClassGUID->Data4[1],
        InterfaceClassGUID->Data4[2], InterfaceClassGUID->Data4[3],
        InterfaceClassGUID->Data4[4], InterfaceClassGUID->Data4[5],
        InterfaceClassGUID->Data4[6], InterfaceClassGUID->Data4[7]
    );

    return STATUS_SUCCESS;
}

void DlWdfRequestComplete(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFREQUEST Request,
    _In_
    NTSTATUS Status
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    
    printf("[INFO] Completing request %p with status 0x%x. This is currently a no-op\n", Request, Status);
}

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
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(Config);
    UNREFERENCED_PARAMETER(QueueAttributes);
    UNREFERENCED_PARAMETER(Queue);

    printf("[INFO] Creating queue is currently a no-op\n");
    return STATUS_SUCCESS;
}

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
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Driver);
    UNREFERENCED_PARAMETER(DesiredAccess);
    UNREFERENCED_PARAMETER(KeyAttributes);

    // This is almost a no-op now
    *Key = (WDFKEY) malloc(1);
    if (*Key == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

VOID DlWdfRegistryClose(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key
)
{
    UNREFERENCED_PARAMETER(DriverGlobals);

    // This is almost a no-op now
    if (Key != NULL) {
        free(Key);
    }
}

NTSTATUS DlWdfRegistryQueryULong(
    _In_
    PWDF_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFKEY Key,
    _In_
    PCUNICODE_STRING ValueName,
    _Out_
    PULONG Value
)
{
#if !defined(_X86_)
    UNICODE_STRING KeyEnableDisplay;
    UNICODE_STRING KeyBootDelay;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (Key == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&KeyEnableDisplay, L"EnableDisplay");
    RtlInitUnicodeString(&KeyBootDelay, L"BootDelay");


    if (RtlEqualUnicodeString(&KeyEnableDisplay, ValueName, FALSE)) {
        printf("[INFO] WdfRegistryQueryUlong requesting EnableDisplay key. Returns 0 to disable HDMI control\n");
        OutputDebugString(L"[INFO] WdfRegistryQueryUlong requesting EnableDisplay key. Returns 0 to disable HDMI control\n");
        *Value = 0;
        return STATUS_SUCCESS;
    }

    if (RtlEqualUnicodeString(&KeyBootDelay, ValueName, FALSE)) {
        printf("[INFO] WdfRegistryQueryUlong requesting BootDelay key. Returns 50 for immediately start\n");
        OutputDebugString(L"[INFO] WdfRegistryQueryUlong requesting BootDelay key. Returns 50 for immediately start\n");
        *Value = 50;
        return STATUS_SUCCESS;
    }
#else
    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Key);
    UNREFERENCED_PARAMETER(ValueName);
    UNREFERENCED_PARAMETER(Value);
#endif

    return STATUS_OBJECT_NAME_NOT_FOUND;
}

NTSTATUS DlWdfUmFunctionImplStub()
{
    // Calling a function that is not yet implemented.
    TrapDebugger("[ERROR] Calling a unimplemented WDF Fx stub function\n");
    return STATUS_NOT_IMPLEMENTED;
}
