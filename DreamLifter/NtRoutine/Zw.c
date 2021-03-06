// Zw.c: some file IO routines

#include <DreamLifter.h>

NTSTATUS DlZwCreateFile(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	PLARGE_INTEGER     AllocationSize,
	ULONG              FileAttributes,
	ULONG              ShareAccess,
	ULONG              CreateDisposition,
	ULONG              CreateOptions,
	PVOID              EaBuffer,
	ULONG              EaLength
)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;

	const wchar_t* devicePrefix = L"\\\\.\\";
	UNICODE_STRING ObjectNameCopy;
	wchar_t ObjectNameCopyBuffer[260];

	UNREFERENCED_PARAMETER(DesiredAccess);
	UNREFERENCED_PARAMETER(IoStatusBlock);
	UNREFERENCED_PARAMETER(AllocationSize);
	UNREFERENCED_PARAMETER(FileAttributes);
	UNREFERENCED_PARAMETER(ShareAccess);
	UNREFERENCED_PARAMETER(CreateDisposition);
	UNREFERENCED_PARAMETER(CreateOptions);
	UNREFERENCED_PARAMETER(EaBuffer);
	UNREFERENCED_PARAMETER(EaLength);

	if (FileHandle == NULL) {
		return STATUS_INVALID_PARAMETER;
	}

	// NT Object:\??\TypeCMux
	// Alias: \DosDevices\TypeCMux
	// UM: Translate it to \\.\TypeCMux
	RtlZeroMemory(ObjectNameCopyBuffer, sizeof(ObjectNameCopyBuffer));
	ObjectNameCopy.Buffer = ObjectNameCopyBuffer;
	ObjectNameCopy.MaximumLength = sizeof(ObjectNameCopyBuffer);
	DlkRtlCopyUnicodeString(&ObjectNameCopy, ObjectAttributes->ObjectName);
	RtlCopyMemory(ObjectNameCopy.Buffer, devicePrefix, 4 * sizeof(wchar_t));
	hFile = CreateFile(
		ObjectNameCopy.Buffer,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		wprintf(L"[WARN] Opening file %s failed\n", ObjectAttributes->ObjectName->Buffer);
		return STATUS_UNSUCESSFUL;
	}

	*FileHandle = hFile;
	return STATUS_SUCCESS;
}

NTSTATUS DlZwClose(
	HANDLE Handle
)
{
	if (Handle != INVALID_HANDLE_VALUE) {
		CloseHandle(Handle);
	}

	return STATUS_SUCCESS;
}