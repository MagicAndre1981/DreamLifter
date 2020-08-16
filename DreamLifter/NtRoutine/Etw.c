// Etw.c: stub ETW implementation

#include <DreamLifter.h>

ULONGLONG g_stubHandle = 114514;

NTSTATUS DlkEtwRegisterClassicProvider(
	LPCGUID ProviderId,
	ULONG Type,
	PVOID EnableCallback,
	PVOID CallbackContext,
	REGHANDLE* RegHandle
)
{
	UNREFERENCED_PARAMETER(ProviderId);
	UNREFERENCED_PARAMETER(Type);
	UNREFERENCED_PARAMETER(EnableCallback);
	UNREFERENCED_PARAMETER(CallbackContext);

	RegHandle = &g_stubHandle;
	return STATUS_SUCCESS;
}

NTSTATUS DlkEtwUnregister(
	REGHANDLE RegHandle
)
{
	UNREFERENCED_PARAMETER(RegHandle);
	return STATUS_SUCCESS;
}
