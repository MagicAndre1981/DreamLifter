// Wmi.c: Windows Managemenet Instrumentation stub implementation

#include <DreamLifter.h>

NTSTATUS DlkWmiTraceMessage(
	TRACEHANDLE LoggerHandle,
	ULONG       MessageFlags,
	LPCGUID     MessageGuid,
	USHORT      MessageNumber,
	...
)
{
	UNREFERENCED_PARAMETER(LoggerHandle);
	UNREFERENCED_PARAMETER(MessageFlags);
	UNREFERENCED_PARAMETER(MessageGuid);
	UNREFERENCED_PARAMETER(MessageNumber);

	return STATUS_SUCCESS;
}


NTSTATUS DlkWmiQueryTraceInformation(
	TRACE_INFORMATION_CLASS TraceInformationClass,
	PVOID                   TraceInformation,
	ULONG                   TraceInformationLength,
	PULONG                  RequiredLength,
	PVOID                   Buffer
)
{
	UNREFERENCED_PARAMETER(TraceInformationClass);
	UNREFERENCED_PARAMETER(TraceInformation);
	UNREFERENCED_PARAMETER(TraceInformationLength);
	UNREFERENCED_PARAMETER(RequiredLength);
	UNREFERENCED_PARAMETER(Buffer);

	return STATUS_NOT_SUPPORTED;
}
