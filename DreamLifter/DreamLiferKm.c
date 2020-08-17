// DreamLiferKm.c: Kernel-mode host

#include <DreamLifter.h>
#include <DreamLifterKmLoader.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;

// HDMIDPI have 5 I2C connections, 1 GPIO connection (unused) and 1 interrupt
CM_PARTIAL_RESOURCE_DESCRIPTOR g_hdmiDpiI2cConnections[5];
DL_WDF_CM_RES_LIST g_hdmiDpiResTranslated;
wchar_t* g_hdmiDpiI2cConnectionStrings[6] = {
	L"\\Device\\RESOURCE_HUB\\0000000000000000",
	L"\\Device\\RESOURCE_HUB\\0000000000000001",
	L"\\Device\\RESOURCE_HUB\\0000000000000002",
	L"\\Device\\RESOURCE_HUB\\0000000000000003",
	L"\\Device\\RESOURCE_HUB\\0000000000000004",
	L""
};

void DlDInitializeHdmiDpiResources()
{
	RtlZeroMemory(g_hdmiDpiI2cConnections, sizeof(g_hdmiDpiI2cConnections));
	RtlZeroMemory(&g_hdmiDpiResTranslated, sizeof(g_hdmiDpiResTranslated));

	for (ULONG i = 0; i < 5; i++)
	{
		g_hdmiDpiI2cConnections[i].Type = CmResourceTypeConnection;
		g_hdmiDpiI2cConnections[i].u.Connection.Class = CM_RESOURCE_CONNECTION_CLASS_SERIAL;
		g_hdmiDpiI2cConnections[i].u.Connection.Type = CM_RESOURCE_CONNECTION_TYPE_SERIAL_I2C;
		// Used as a placeholder, identified by the kernel client by sequence
		g_hdmiDpiI2cConnections[i].u.Connection.IdLowPart = i;
		g_hdmiDpiI2cConnections[i].u.Connection.IdHighPart = 0;
	}
	
	g_hdmiDpiResTranslated.Header.Magic = DREAMLIFTER_OBJECT_HEADER_MAGIC;
	g_hdmiDpiResTranslated.Header.Type = DlObjectTypeCmListTranslated;
	g_hdmiDpiResTranslated.ResourceCount = 5;
	g_hdmiDpiResTranslated.Resources = g_hdmiDpiI2cConnections;
}

int DlStartKmHost()
{
	PDRIVER_MODULE driverModule = DlKmLoadModule();
	PVOID pDriverObjectStub = (PVOID)driverModule;
	UNICODE_STRING FakeRegString;

	NTSTATUS status = STATUS_SUCCESS;
	int err = 0;

	DlFxLdrGetWdfKmdf0115Functions();
	DlDInitializeHdmiDpiResources();

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

	// Invoke:
	// - DeviceAdd
	// - PrepareHardware
	// - Enter D0
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

	if (g_pDevice != NULL && g_pDevice->EvtDevicePrepareHardware != NULL) {
		status = g_pDevice->EvtDevicePrepareHardware((WDFDEVICE)g_pDevice, (WDFCMRESLIST)&g_hdmiDpiResTranslated, (WDFCMRESLIST)&g_hdmiDpiResTranslated);
		if (!NT_SUCCESS(status)) {
			printf("[ERROR] EvtDevicePrepareHardware failed: 0x%x\n", status);
			err = RtlNtStatusToDosError(status);
			goto exit;
		}
	}

exit:
	return err;
}

