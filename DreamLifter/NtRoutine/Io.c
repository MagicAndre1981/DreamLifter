// Io.c: IO routines

#include <DreamLifter.h>

NTSTATUS DlkIoWMIRegistrationControl(
	PVOID          DeviceObject,
	ULONG          Action
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Action);

	return STATUS_SUCCESS;
}
