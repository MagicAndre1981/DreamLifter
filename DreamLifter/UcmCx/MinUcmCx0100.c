// MinUcmCx0100.c: UCM CX 1.00 min implementation

#include <DreamLifter.h>

extern PDRIVER_INSTANCE g_pDriverInstance;
extern PDREAMLIFTER_DEVICE g_pDevice;

NTSTATUS DlUcmInitializeDevice(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_MANAGER_CONFIG Config
)
{
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE) WdfDevice;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Config);

    if (WdfDevice == NULL || Config == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pDevice->UcmManagerInfo == NULL) {
        pDevice->UcmManagerInfo = malloc(sizeof(DREAMLIFTER_UCM_DEVICE));
        if (pDevice->UcmManagerInfo == NULL) {
            printf("[ERROR] Failed to allocate memory for UCM device\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        printf("[INFO] UcmCx initializes the manager instance\n");
        return STATUS_SUCCESS;
    }

    // Manager is expected to initialize exactly once
    return STATUS_INVALID_HANDLE;
}

NTSTATUS DlUcmCreateConnector(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    WDFDEVICE WdfDevice,
    _In_
    PUCM_CONNECTOR_CONFIG Config,
    _In_opt_
    PWDF_OBJECT_ATTRIBUTES Attributes,
    _Out_
    UCMCONNECTOR* Connector
)
{
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE) WdfDevice;

    UNREFERENCED_PARAMETER(DriverGlobals);
    UNREFERENCED_PARAMETER(Attributes);

    if (WdfDevice == NULL || Config == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pDevice->UcmManagerInfo == NULL) {
        // Manager is not yet initialized
        return STATUS_INVALID_HANDLE;
    }

    pDevice->UcmManagerInfo->ConnectorId = Config->ConnectorId;
    RtlCopyMemory((PVOID) &pDevice->UcmManagerInfo->PdConfig, (PVOID) Config->PdConfig, sizeof(UCM_CONNECTOR_PD_CONFIG));
    RtlCopyMemory((PVOID) &pDevice->UcmManagerInfo->TypeCConfig, (PVOID)Config->TypeCConfig, sizeof(UCM_CONNECTOR_TYPEC_CONFIG));
    *Connector = (UCMCONNECTOR)pDevice->UcmManagerInfo;

    printf("[INFO] Type-C connector summary: supported modes 0x%x, power source capabilities 0x%x, analog audio 0x%x \n",
        pDevice->UcmManagerInfo->TypeCConfig.SupportedOperatingModes,
        pDevice->UcmManagerInfo->TypeCConfig.SupportedPowerSourcingCapabilities,
        pDevice->UcmManagerInfo->TypeCConfig.AudioAccessoryCapable
    );

    printf("[INFO] USB-PD summary: support 0x%x, suppoted power roles 0x%x \n",
        pDevice->UcmManagerInfo->PdConfig.IsSupported,
        pDevice->UcmManagerInfo->PdConfig.SupportedPowerRoles
    );

    return STATUS_SUCCESS;
}
