// KmdfLoader.c: handles KMDF load

#include <DreamLifter.h>
#include <wdf/Km/KmdfDriverEnums0115.h>

PVOID g_WdfFunctions0115[WdfFunctionTableNumEntries];

PVOID DlFxLdrGetWdfKmdf0115Functions()
{
	RtlZeroMemory(g_WdfFunctions0115, sizeof(g_WdfFunctions0115));

	for (ULONG i = 0; i < WdfFunctionTableNumEntries; i++) {
		g_WdfFunctions0115[i] = (PVOID)DlWdfKmFunctionImplStub;
	}

	g_WdfFunctions0115[WdfDriverCreateTableIndex] = (PVOID)DlWdfCreateDriver;
	g_WdfFunctions0115[WdfControlDeviceInitAllocateTableIndex] = (PVOID)DlWdfControlDeviceInitAllocate;
	g_WdfFunctions0115[WdfDeviceInitSetPnpPowerEventCallbacksTableIndex] = (PVOID)DlWdfDeviceInitSetPnpPowerEventCallbacks;
	g_WdfFunctions0115[WdfDeviceInitSetFileObjectConfigTableIndex] = (PVOID)DlWdfDeviceInitSetFileObjectConfig;
	g_WdfFunctions0115[WdfDeviceCreateTableIndex] = (PVOID)DlWdfDeviceCreate;
	g_WdfFunctions0115[WdfDeviceCreateDeviceInterfaceTableIndex] = (PVOID)DlWdfCreateDeviceInterface;
	g_WdfFunctions0115[WdfDeviceCreateSymbolicLinkTableIndex] = (PVOID)DlWdfDeviceCreateSymbolicLink;

	g_WdfFunctions0115[WdfObjectGetTypedContextWorkerTableIndex] = (PVOID) DlWdfObjectGetTypedContextWorker;

	g_WdfFunctions0115[WdfSpinLockCreateTableIndex] = (PVOID)DlWdfSpinLockCreate;
	g_WdfFunctions0115[WdfSpinLockAcquireTableIndex] = (PVOID)DlWdfSpinLockAcquire;
	g_WdfFunctions0115[WdfSpinLockReleaseTableIndex] = (PVOID)DlWdfSpinLockRelease;

	g_WdfFunctions0115[WdfTimerCreateTableIndex] = (PVOID)DlWdfTimerCreate;
	g_WdfFunctions0115[WdfTimerGetParentObjectTableIndex] = (PVOID)DlWdfTimerGetParentObject;
	g_WdfFunctions0115[WdfTimerStartTableIndex] = (PVOID)DlWdfTimerStart;
	g_WdfFunctions0115[WdfTimerStopTableIndex] = (PVOID)DlWdfTimerStop;

	g_WdfFunctions0115[WdfWorkItemCreateTableIndex] = (PVOID)DlWdfWorkItemCreate;
	g_WdfFunctions0115[WdfWorkItemGetParentObjectTableIndex] = (PVOID)DlWdfWorkItemGetParentObject;
	g_WdfFunctions0115[WdfWorkItemEnqueueTableIndex] = (PVOID)DlWdfWorkItemEnqueue;

	g_WdfFunctions0115[WdfInterruptCreateTableIndex] = (PVOID) DlWdfInterruptCreate;
	g_WdfFunctions0115[WdfInterruptEnableTableIndex] = (PVOID) DlWdfInterruptEnable;
	g_WdfFunctions0115[WdfInterruptDisableTableIndex] = (PVOID) DlWdfInterruptDisable;

	g_WdfFunctions0115[WdfIoQueueCreateTableIndex] = (PVOID) DlWdfIoQueueCreate;
	g_WdfFunctions0115[WdfIoTargetCreateTableIndex] = (PVOID) DlWdfIoTargetCreate;
	g_WdfFunctions0115[WdfIoTargetOpenTableIndex] = (PVOID) DlWdfIoTargetOpen;
	g_WdfFunctions0115[WdfIoTargetCloseTableIndex] = (PVOID) DlWdfIoTargetClose;

	g_WdfFunctions0115[WdfDeviceWdmGetDeviceObjectTableIndex] = (PVOID) DlWdfDeviceWdmGetDeviceObject;

	g_WdfFunctions0115[WdfCmResourceListGetCountTableIndex] = (PVOID)DlWdfCmResourceListGetCount;
	g_WdfFunctions0115[WdfCmResourceListGetDescriptorTableIndex] = (PVOID)DlWdfCmResourceListGetDescriptor;

	g_WdfFunctions0115[WdfObjectDeleteTableIndex] = (PVOID) DlWdfObjectDelete;

	return (PVOID)g_WdfFunctions0115;
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
