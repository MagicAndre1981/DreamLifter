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

    printf("[INFO] Type-C connector summary: supported modes %s, power source capabilities 0x%x, analog audio 0x%x\n",
        DbgUcmGetOperatingMode(pDevice->UcmManagerInfo->TypeCConfig.SupportedOperatingModes),
        pDevice->UcmManagerInfo->TypeCConfig.SupportedPowerSourcingCapabilities,
        pDevice->UcmManagerInfo->TypeCConfig.AudioAccessoryCapable
    );

    printf("[INFO] USB-PD summary: support 0x%x, suppoted power roles 0x%x\n",
        pDevice->UcmManagerInfo->PdConfig.IsSupported,
        pDevice->UcmManagerInfo->PdConfig.SupportedPowerRoles
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorTypeCAttach(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_TYPEC_ATTACH_PARAMS Params
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE) Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->Connected = TRUE;
        pConnector->ChargingState = Params->ChargingState;
        pConnector->PowerCurrent = Params->CurrentAdvertisement;
        pConnector->Partner = Params->Partner;
    }

    printf("[INFO] Ucm reports Type-C attached. Charging state %s, current %s, partner %s\n",
        DbgUcmGetChargingState(pConnector->ChargingState),
        DbgUcmGetCurrent(pConnector->PowerCurrent),
        DbgUcmGetPartner(pConnector->Partner)
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorTypeCDetach(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->Connected = FALSE;

        pConnector->ChargingState = UcmChargingStateInvalid;
        pConnector->PdConnState = UcmPdConnStateInvalid;
        pConnector->PowerCurrent = UcmTypeCCurrentInvalid; 

        pConnector->Partner = UcmTypeCPartnerInvalid;
        pConnector->PowerRole = UcmPowerRoleInvalid;
        pConnector->DataRole = UcmDataRoleInvalid;

        pConnector->SourcePdoCount = 0;
        pConnector->PartnerPdoCount = 0;
        RtlZeroMemory(pConnector->SourcePdos, sizeof(pConnector->SourcePdos));
        RtlZeroMemory(pConnector->PartnerPdos, sizeof(pConnector->PartnerPdos));
        RtlZeroMemory(&pConnector->PdRdo, sizeof(UCM_PD_REQUEST_DATA_OBJECT));
    }

    printf("[INFO] Ucm reports Type-C detached\n");
    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorTypeCCurrentAdChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_TYPEC_CURRENT CurrentAdvertisement
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->PowerCurrent = CurrentAdvertisement;
    }

    printf("[INFO] Ucm reports power current change: %s\n",
        DbgUcmGetCurrent(pConnector->PowerCurrent)
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorPdSourceCaps(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->SourcePdoCount = PdoCount;
        RtlZeroMemory(pConnector->SourcePdos, sizeof(pConnector->SourcePdos));
        RtlCopyMemory(pConnector->SourcePdos, Pdos, sizeof(UCM_PD_POWER_DATA_OBJECT) * PdoCount);
    }

    printf("[INFO] Ucm receives new source power capabilities\n");
    for (UCHAR i = 0; i < PdoCount; i++) {
        UCM_PD_POWER_DATA_OBJECT Pdo = Pdos[i];
        printf("Source power PDO %d:\n", i);
        DbgUcmDumpPdo(Pdo);
        printf("\n");
    }

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorPdPartnerSourceCaps(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_PD_POWER_DATA_OBJECT Pdos[],
    _In_
    UCHAR PdoCount
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->PartnerPdoCount = PdoCount;
        RtlZeroMemory(pConnector->PartnerPdos, sizeof(pConnector->PartnerPdos));
        RtlCopyMemory(pConnector->PartnerPdos, Pdos, sizeof(UCM_PD_POWER_DATA_OBJECT) * PdoCount);
    }

    printf("[INFO] Ucm receives new partner power capabilities\n");
    for (UCHAR i = 0; i < PdoCount; i++) {
        UCM_PD_POWER_DATA_OBJECT Pdo = Pdos[i];
        printf("Partner power PDO %d:\n", i);
        DbgUcmDumpPdo(Pdo);
        printf("\n");
    }

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorPdConnectionStateChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    PUCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS Params
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->Connected = TRUE;
        pConnector->ChargingState = Params->ChargingState;
        pConnector->PdConnState = Params->PdConnState;
        RtlCopyMemory(&pConnector->PdRdo, &Params->Rdo, sizeof(UCM_PD_REQUEST_DATA_OBJECT));
    }

    printf("[INFO] Ucm reports PD state change: PD state %s, Charging state %s\n",
        DbgUcmGetPdConnState(pConnector->PdConnState),
        DbgUcmGetChargingState(pConnector->ChargingState)
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorChargingStateChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    UCM_CHARGING_STATE ChargingState
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->ChargingState = ChargingState;
    }

    printf("[INFO] Ucm reports charging state change: %s\n",
        DbgUcmGetChargingState(pConnector->ChargingState)
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorDataDirectionChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_DATA_ROLE CurrentDataRole
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->DataRole = CurrentDataRole;
    }

    printf("[INFO] Ucm reports data role change: %s. Change state 0x%x.\n",
        DbgUcmGetDataRole(pConnector->DataRole),
        Success
    );

    return STATUS_SUCCESS;
}

NTSTATUS DlUcmConnectorPowerDirectionChanged(
    _In_
    PUCM_DRIVER_GLOBALS DriverGlobals,
    _In_
    UCMCONNECTOR Connector,
    _In_
    BOOLEAN Success,
    _In_
    UCM_POWER_ROLE CurrentPowerRole
)
{
    PDREAMLIFTER_UCM_DEVICE pConnector = (PDREAMLIFTER_UCM_DEVICE)Connector;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->PowerRole = CurrentPowerRole;
    }

    printf("[INFO] Ucm reports power role change: %s. Change state 0x%x.\n",
        DbgUcmGetPowerRole(pConnector->PowerRole),
        Success
    );

    return STATUS_SUCCESS;
}
