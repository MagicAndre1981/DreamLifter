// MinKmdf0115.c: KMDF 1.15 minimum implementation

#include <DreamLifter.h>
#include <wdf/Km/KmdfDriverEnums0115.h>

NTSTATUS DlWdfKmFunctionImplStub()
{
	// Calling a function that is not yet implemented.
	printf("[ERROR] Calling a unimplemented WDF KM Fx stub function\n");

	if (IsDebuggerPresent())
	{
		DebugBreak();
	}

	return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS DlWdfVersionUnbind()
{
	return STATUS_SUCCESS;
}

NTSTATUS DlWdfVersionUnbindClass()
{
	return STATUS_SUCCESS;
}

PWDFDEVICE_INIT DlWdfControlDeviceInitAllocate(
	PWDF_DRIVER_GLOBALS  DriverGlobals,
	WDFDRIVER            Driver,
	const UNICODE_STRING* SDDLString
)
{
	PDREAMLIFTER_DEVICE_INIT pDlDriverInit = NULL;

	UNREFERENCED_PARAMETER(Driver);
	UNREFERENCED_PARAMETER(DriverGlobals);

	if (SDDLString->Buffer != NULL) {
		// Basically this is a no-op now, we returns an empty init allocate thing
		wprintf(L"[INFO] Requested WdfControlDeviceInitAllocate with SDDL %s. DreamLifter doesn't support access control yet\n", SDDLString->Buffer);
	}

	pDlDriverInit = malloc(sizeof(DREAMLIFTER_DEVICE_INIT));
	if (pDlDriverInit != NULL) {
		RtlZeroMemory(pDlDriverInit, sizeof(DREAMLIFTER_DEVICE_INIT));
	}
	else {
		printf("[ERROR] Failed to allocate Device Init struct\n");
		__fastfail(FAST_FAIL_UNEXPECTED_HOST_BEHAVIOR);
	}

	return (PWDFDEVICE_INIT)pDlDriverInit;
}
