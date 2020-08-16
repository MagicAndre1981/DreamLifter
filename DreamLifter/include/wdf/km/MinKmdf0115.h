#pragma once

#ifndef __MIN_KMDF_1_15_H__
#define __MIN_KMDF_1_15_H__

void DlKmImplementationStub();

NTSTATUS
DlKWdfVersionBind(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath,
	_Inout_ PWDF_BIND_INFO BindInfo,
	_Out_ PWDF_COMPONENT_GLOBALS*
	ComponentGlobals
);

NTSTATUS DlKmWdfVersionBindClass(
	_In_ PVOID Context,
	_In_ PWDF_BIND_INFO BindInfo,
	_In_ PWDF_DRIVER_GLOBALS DriverGlobals,
	_In_ PWDF_CLASS_EXTENSION ClassExtensionInfo
);

NTSTATUS DlWdfVersionUnbind();
NTSTATUS DlWdfVersionUnbindClass();

NTSTATUS DlWdfKmFunctionImplStub();
void DlKmImplementationStubNoOp();
PVOID DlFxLdrGetWdfKmdf0115Functions();

PWDFDEVICE_INIT DlWdfControlDeviceInitAllocate(
	PWDF_DRIVER_GLOBALS DriverGlobals,
	WDFDRIVER            Driver,
	const UNICODE_STRING* SDDLString
);

PDEVICE_OBJECT DlWdfDeviceWdmGetDeviceObject(
	PWDF_DRIVER_GLOBALS  DriverGlobals,
	WDFDEVICE Device
);

#endif
