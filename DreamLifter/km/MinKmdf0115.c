// MinKmdf0115.c: KMDF 1.15 minimum implementation

#include <DreamLifter.h>
#include <wdf/Km/KmdfDriverEnums0115.h>

PVOID g_WdfFunctions0115[WdfFunctionTableNumEntries];

PVOID DlFxLdrGetWdfKmdf0115Functions()
{
    RtlZeroMemory(g_WdfFunctions0115, sizeof(g_WdfFunctions0115));

    for (ULONG i = 0; i < WdfFunctionTableNumEntries; i++) {
        g_WdfFunctions0115[i] = (PVOID) DlWdfKmFunctionImplStub;
    }

    return (PVOID) g_WdfFunctions0115;
}

NTSTATUS
DlKWdfVersionBind(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath,
	_Inout_ PWDF_BIND_INFO BindInfo,
	_Out_ PWDF_COMPONENT_GLOBALS* ComponentGlobals
)
{
	PWDF_DRIVER_GLOBALS DriverGlobals = NULL;

	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	if (BindInfo->Version.Major == 1 && BindInfo->Version.Minor == 15) {
		printf("[INFO] Request WDF KMDF version 1.15\n");

		if (BindInfo->FuncCount == WdfFunctionTableNumEntries)
		{
			DriverGlobals = malloc(sizeof(WDF_DRIVER_GLOBALS));
			if (DriverGlobals == NULL) {
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			// Set some fields. Can't set all yet
			RtlCopyMemory(DriverGlobals->DriverName, GUEST_DRIVER_NAME_KM, sizeof(GUEST_DRIVER_NAME_KM));
			DriverGlobals->DisplaceDriverUnload = FALSE;

			*BindInfo->FuncTable = g_WdfFunctions0115;
			*ComponentGlobals = (PVOID)DriverGlobals;

			return STATUS_SUCCESS;
		}
		else
		{
			printf("[INFO] WDF function table entry count mismatch\n");
			return STATUS_INVALID_PARAMETER;
		}
	}
	
	return STATUS_NOT_SUPPORTED;
}

NTSTATUS DlKmWdfVersionBindClass(
	_In_ PVOID Context,
	_In_ PWDF_BIND_INFO BindInfo,
	_In_ PWDF_DRIVER_GLOBALS DriverGlobals,
	_In_ PWDF_CLASS_EXTENSION ClassExtensionInfo
)
{
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(BindInfo);
	UNREFERENCED_PARAMETER(DriverGlobals);
	UNREFERENCED_PARAMETER(ClassExtensionInfo);

	printf("[WARN] WDF KMDF Class Extension is not yet supported\n");
	return STATUS_NOT_SUPPORTED;
}

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
