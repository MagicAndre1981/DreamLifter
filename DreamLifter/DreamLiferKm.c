// DreamLiferKm.c: Kernel-mode host

#include <DreamLifter.h>
#include <DreamLifterKmLoader.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;

int DlStartKmHost()
{
	PDRIVER_MODULE driverModule = DlKmLoadModule();
	PVOID pDriverObjectStub = (PVOID)driverModule;
	UNICODE_STRING FakeRegString;

	NTSTATUS status = STATUS_SUCCESS;
	int err = 0;

	DlFxLdrGetWdfKmdf0115Functions();

	RtlInitUnicodeString(&FakeRegString, L"HKLM\\Something");
	if (driverModule != NULL && driverModule->Entry != NULL) {
		status = driverModule->Entry(pDriverObjectStub, &FakeRegString);
		if (!NT_SUCCESS(status)) {
			printf("[ERROR] DriverEntry failed: 0x%x\n", status);
			err = RtlNtStatusToDosError(status);
			goto exit;
		}
	}
	else {
		printf("[ERROR] Unable to load driver file\n");
		err = 1;
		goto exit;
	}

	if (g_pDriverInstance->DriverDeviceAdd != NULL) {
		DREAMLIFTER_DEVICE_INIT deviceInit;
		RtlZeroMemory(&deviceInit, sizeof(deviceInit));
		deviceInit.Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
		deviceInit.Header.Type = DlObjectTypeDeviceInit;

		status = g_pDriverInstance->DriverDeviceAdd((WDFDRIVER)g_pDriverInstance, (PWDFDEVICE_INIT)&deviceInit);
		if (!NT_SUCCESS(status)) {
			printf("[ERROR] DriverDeviceAdd failed: 0x%x\n", status);
			err = RtlNtStatusToDosError(status);
			goto exit;
		}
	}

exit:
	return err;
}

