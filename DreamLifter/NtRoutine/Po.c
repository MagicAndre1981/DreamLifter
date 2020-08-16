// Po.c: Power Management wrapper

#include <DreamLifter.h>

NTSTATUS DlPoRegisterPowerSettingCallback(
	PDEVICE_OBJECT          DeviceObject,
	LPCGUID                 SettingGuid,
	PPOWER_SETTING_CALLBACK Callback,
	PVOID                   Context,
	PVOID* Handle
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(SettingGuid);
	UNREFERENCED_PARAMETER(Callback);
	UNREFERENCED_PARAMETER(Context);
	UNREFERENCED_PARAMETER(Handle);

	// TODO: connect this callback with kernel-mode helper

	return STATUS_SUCCESS;
}
