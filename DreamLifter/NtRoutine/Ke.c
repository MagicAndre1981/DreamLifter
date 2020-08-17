// Ke.c: Kernel Executives

#include <DreamLifter.h>

UINT8 DlKeGetCurrentIrql()
{
	// Always passive level in um
	return 0;
}

void DlKeInitializeMutex(
	PRKMUTEX Mutex,
	ULONG    Level
)
{
	HANDLE hMutex = INVALID_HANDLE_VALUE;

	UNREFERENCED_PARAMETER(Level);
	if (Mutex == NULL) {
		TrapDebugger("[ERROR] Pass null pointer of mutex struct in DlKeInitializeMutex\n");
		CpuDeadLoop();
		return;
	}

	// Default is signaled / not-owned
	hMutex = CreateMutex(NULL, FALSE, NULL);
	if (hMutex == INVALID_HANDLE_VALUE) {
		TrapDebugger("[ERROR] Failed to create mutex\n");
		CpuDeadLoop();
		return;
	}

	// We will use this to store the handle
	Mutex->Header.WaitListHead.Flink = (PVOID)hMutex;
	Mutex->Header.Type = 0;
}

LONG DlKeReleaseMutex(
	PRKMUTEX Mutex,
	BOOLEAN  Wait
)
{
	UNREFERENCED_PARAMETER(Wait);

	if (ReleaseMutex((HANDLE)Mutex->MutantListEntry.Flink)) {
		return 0;
	}

	return 1;
}

void DlKeInitializeEvent(
	PRKEVENT   Event,
	EVENT_TYPE Type,
	BOOLEAN    State
)
{
	HANDLE hEvent = INVALID_HANDLE_VALUE;
	DWORD Flags = 0;

	UNREFERENCED_PARAMETER(Type);
	if (Event == NULL) {
		TrapDebugger("[ERROR] Pass null pointer of mutex struct in DlKeInitializeMutex\n");
		CpuDeadLoop();
		return;
	}

	if (State) {
		State |= CREATE_EVENT_INITIAL_SET;
	}

	if (Type == NotificationEvent) {
		State |= CREATE_EVENT_MANUAL_RESET;
	}

	hEvent = CreateEventEx(
		NULL,
		NULL,
		Flags,
		EVENT_ALL_ACCESS
	);

	if (hEvent == INVALID_HANDLE_VALUE) {
		TrapDebugger("[ERROR] Failed to create event\n");
		CpuDeadLoop();
		return;
	}

	// We will use this to store the handle
	Event->Header.WaitListHead.Flink = (PVOID) hEvent;
	Event->Header.Type = 1;
}

void DlKeClearEvent(
	PRKEVENT Event
)
{
	if (Event->Header.Type == 1) {
		ResetEvent((HANDLE) Event->Header.WaitListHead.Flink);
	}
}

LONG DlKeSetEvent(
	PRKEVENT  Event,
	KPRIORITY Increment,
	BOOLEAN   Wait
)
{
	UNREFERENCED_PARAMETER(Increment);
	UNREFERENCED_PARAMETER(Wait);

	LONG prevState = DlKeReadStateEvent(Event);

	if (Event->Header.Type == 1) {
		SetEvent((HANDLE) Event->Header.WaitListHead.Flink);
	}

	return prevState;
}

LONG DlKeReadStateEvent(
	PRKEVENT Event
)
{
	if (Event->Header.Type == 1) {
		DWORD ret = WaitForSingleObjectEx((HANDLE)Event->Header.WaitListHead.Flink, 0, FALSE);
		if (ret == WAIT_OBJECT_0) {
			return 1;
		}
	}

	return 0;
}

NTSTATUS
DlKeWaitForSingleObject(
	PVOID Object,
	KWAIT_REASON WaitReason,
	KPROCESSOR_MODE WaitMode,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout
)
{
	PDISPATCHER_HEADER hHeader = (PDISPATCHER_HEADER) Object;
	DWORD timeOut = 0;
	DWORD ret = 0;

	UNREFERENCED_PARAMETER(WaitReason);
	UNREFERENCED_PARAMETER(WaitMode);

	if (hHeader == NULL || hHeader->Type > 1) {
		return STATUS_INVALID_PARAMETER;
	}

	if (Timeout == NULL) {
		timeOut = INFINITE;
	}
	else if (Timeout->QuadPart < 0) {
		timeOut = (DWORD)((-Timeout->QuadPart * 100) / 1000000);
	}
	else {
		TrapDebugger("[ERROR] Absolute time is not yet supported in DlKeWaitForSingleObject\n");
		return STATUS_NOT_SUPPORTED;
	}

	ret = WaitForSingleObjectEx((HANDLE)hHeader->WaitListHead.Flink, timeOut, Alertable);
	switch (ret) {
	case 0x00000000L:
		return STATUS_SUCCESS;
	case 0x00000102L:
		return STATUS_TIMEOUT;
	}

	return STATUS_TIMEOUT;
}
