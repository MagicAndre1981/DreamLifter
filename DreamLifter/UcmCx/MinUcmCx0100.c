// MinUcmCx0100.c: UCM CX 1.00 min implementation

#include <DreamLifter.h>

#define DriverProxyPath L"\\\\.\\DlUcmProxyDevice"

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

        // Open the driver component
        pDevice->UcmManagerInfo->ProxyDriverHandle = CreateFile(
            DriverProxyPath,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
        );

        if (pDevice->UcmManagerInfo->ProxyDriverHandle == NULL) {
            printf("[ERROR] UcmCx failed to open the driver component. Error %d\n", GetLastError());
            return STATUS_OBJECT_NAME_NOT_FOUND;
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
    HANDLE h;

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

    // Create two threads to poll the state
    h = CreateThread(
        NULL,
        0,
        DlUcmPowerRoleEventWorker,
        pDevice,
        0,
        NULL
    );

    if (h == NULL) {
        printf("[ERROR] Failed to start polling state\n");
        return STATUS_UNSUCESSFUL;
    }

    h = CreateThread(
        NULL,
        0,
        DlUcmDataRoleEventWorker,
        pDevice,
        0,
        NULL
    );

    if (h == NULL) {
        printf("[ERROR] Failed to start polling state\n");
        return STATUS_UNSUCESSFUL;
    }

    printf("[INFO] Proxy event polling has started\n");

    return STATUS_SUCCESS;
}

DWORD WINAPI DlUcmPowerRoleEventWorker(
    LPVOID lpParam
)
{
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE) lpParam;
    UCHAR Role = 0;
    BOOL ret;
    NTSTATUS status;
    HANDLE DeviceHandle;

    DeviceHandle = CreateFile(
        DriverProxyPath,
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (DeviceHandle == NULL) {
        printf("[ERROR] UcmCx failed to open the driver component. Error %d\n", GetLastError());
        return 6;
    }

    while (TRUE) {
        ret = DeviceIoControl(DeviceHandle, IOCTL_UCMPROXY_WAIT_SET_POWER_ROLE_CALLBACK,
            NULL, 0,
            (LPVOID) &Role, sizeof(UCHAR),
            NULL, NULL
        );

        if (ret) {
            printf("[INFO] EvtSetPowerRole is requested. New Role %s (%d). Event called.\n", DbgUcmGetPowerRole((UCM_POWER_ROLE)Role), Role);
            if (pDevice->UcmManagerInfo->PdConfig.EvtSetPowerRole != NULL) {
                printf("[INFO] EvtSetPowerRole is firing.\n");
                status = pDevice->UcmManagerInfo->PdConfig.EvtSetPowerRole((UCMCONNECTOR) pDevice->UcmManagerInfo, (UCM_POWER_ROLE) Role);
                if (!NT_SUCCESS(status)) {
                    printf("[ERROR] EvtSetPowerRole failed 0x%x\n", status);
                }
            }
        }
        else {
            printf("[ERROR] DlUcmPowerRoleEventWorker Proxy component reports failure, error %d\n", GetLastError());
            break;
        }

        Role = 0;
    }

    return 0;
}

DWORD WINAPI DlUcmDataRoleEventWorker(
    LPVOID lpParam
)
{
    PDREAMLIFTER_DEVICE pDevice = (PDREAMLIFTER_DEVICE)lpParam;
    UCHAR Role = 0;
    BOOL ret;
    NTSTATUS status;
    HANDLE DeviceHandle;

    DeviceHandle = CreateFile(
        DriverProxyPath,
        FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (DeviceHandle == NULL) {
        printf("[ERROR] UcmCx failed to open the driver component. Error %d\n", GetLastError());
        return 6;
    }

    while (TRUE) {
        ret = DeviceIoControl(DeviceHandle, IOCTL_UCMPROXY_WAIT_SET_DATA_ROLE_CALLBACK,
            NULL, 0,
            (LPVOID)&Role, sizeof(UCHAR),
            NULL, NULL
        );

        if (ret) {
            printf("[INFO] EvtSetDataRole is requested. New Role %s (%d)\n", DbgUcmGetDataRole((UCM_POWER_ROLE)Role), Role);
            if (pDevice->UcmManagerInfo->TypeCConfig.EvtSetDataRole != NULL) {
                printf("[INFO] EvtSetDataRole is firing.\n");
                status = pDevice->UcmManagerInfo->TypeCConfig.EvtSetDataRole((UCMCONNECTOR)pDevice->UcmManagerInfo, (UCM_POWER_ROLE)Role);
                if (!NT_SUCCESS(status)) {
                    printf("[ERROR] EvtSetDataRole failed 0x%x\n", status);
                }
            }
        }
        else {
            printf("[ERROR] DlUcmDataRoleEventWorker Proxy component reports failure, error %d\n", GetLastError());
            break;
        }

        Role = 0;
    }

    return 0;
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
    BOOL ret;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->Connected = TRUE;
        pConnector->ChargingState = Params->ChargingState;
        pConnector->PowerCurrent = Params->CurrentAdvertisement;
        pConnector->Partner = Params->Partner;
    }

    printf("[INFO] Ucm reports Type-C attached. Charging state %s (%d), current %s (%d), partner %s (%d)\n",
        DbgUcmGetChargingState(pConnector->ChargingState),
        pConnector->ChargingState,
        DbgUcmGetCurrent(pConnector->PowerCurrent),
        pConnector->PowerCurrent,
        DbgUcmGetPartner(pConnector->Partner),
        pConnector->Partner
    );

    Params->Size = sizeof(UCM_CONNECTOR_TYPEC_ATTACH_PARAMS);
    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_ATTACH,
        (LPVOID) Params, sizeof(UCM_CONNECTOR_TYPEC_ATTACH_PARAMS),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
    }

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
    UCHAR stub = 0;
    BOOL ret;

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

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_DETACH,
        (LPVOID) &stub, sizeof(UCHAR),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
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
    BOOL ret;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->PowerCurrent = CurrentAdvertisement;
    }

    printf("[INFO] Ucm reports power current change: %s (%d)\n",
        DbgUcmGetCurrent(pConnector->PowerCurrent),
        pConnector->PowerCurrent
    );

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_CURRENT_CHANGE,
        (LPVOID) &((UCHAR) CurrentAdvertisement), sizeof(UCHAR),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
    }

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
    PROXY_POWER_CAPS_EXCHANGE_REQUEST Request;
    BOOL ret;

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

    RtlZeroMemory(&Request, sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST));

    Request.Size = sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST);
    RtlCopyMemory(&Request.Pdos, Pdos, sizeof(UCM_PD_POWER_DATA_OBJECT) * PdoCount);
    Request.PdoCount = PdoCount;

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_PD_SOURCE_CAPS,
        (LPVOID) &Request, sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
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
    PROXY_POWER_CAPS_EXCHANGE_REQUEST Request;
    BOOL ret;

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

    RtlZeroMemory(&Request, sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST));

    Request.Size = sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST);
    RtlCopyMemory(&Request.Pdos, Pdos, sizeof(UCM_PD_POWER_DATA_OBJECT) * PdoCount);
    Request.PdoCount = PdoCount;

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_PD_PARTNER_CAPS,
        (LPVOID)&Request, sizeof(PROXY_POWER_CAPS_EXCHANGE_REQUEST),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
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
    BOOL ret;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->Connected = TRUE;
        pConnector->ChargingState = Params->ChargingState;
        pConnector->PdConnState = Params->PdConnState;
        RtlCopyMemory(&pConnector->PdRdo, &Params->Rdo, sizeof(UCM_PD_REQUEST_DATA_OBJECT));
    }

    printf("[INFO] Ucm reports PD state change: PD state %s (%d), Charging state %s (%d)\n",
        DbgUcmGetPdConnState(pConnector->PdConnState),
        pConnector->PdConnState,
        DbgUcmGetChargingState(pConnector->ChargingState),
        pConnector->ChargingState
    );

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_PD_CONN_STATE_CHANGE,
        (LPVOID) &Params, sizeof(UCM_CONNECTOR_PD_CONN_STATE_CHANGED_PARAMS),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
    }

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
    BOOL ret;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->ChargingState = ChargingState;
    }

    printf("[INFO] Ucm reports charging state change: %s (%d)\n",
        DbgUcmGetChargingState(pConnector->ChargingState),
        pConnector->ChargingState
    );

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_CHARGING_STATE_CHANGE,
        (LPVOID) &((UCHAR)ChargingState), sizeof(UCHAR),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
    }

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
    PROXY_ROLE_CHANGE_REQUEST Request;
    BOOL ret;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->DataRole = CurrentDataRole;
    }

    printf("[INFO] Ucm reports data role change: %s (%d). Change state 0x%x.\n",
        DbgUcmGetDataRole(pConnector->DataRole),
        pConnector->DataRole,
        Success
    );

    RtlZeroMemory(&Request, sizeof(PROXY_ROLE_CHANGE_REQUEST));
    Request.Role = CurrentDataRole;
    Request.Success = Success;

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_DATA_DIRECTION_CHANGE,
        (LPVOID)&Request, sizeof(PROXY_ROLE_CHANGE_REQUEST),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
    }

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
    PROXY_ROLE_CHANGE_REQUEST Request;
    BOOL ret;

    UNREFERENCED_PARAMETER(DriverGlobals);

    if (pConnector != NULL) {
        pConnector->PowerRole = CurrentPowerRole;
    }

    printf("[INFO] Ucm reports power role change: %s (%d). Change state 0x%x.\n",
        DbgUcmGetPowerRole(pConnector->PowerRole),
        pConnector->PowerRole,
        Success
    );

    RtlZeroMemory(&Request, sizeof(PROXY_ROLE_CHANGE_REQUEST));
    Request.Role = CurrentPowerRole;
    Request.Success = Success;

    ret = DeviceIoControl(pConnector->ProxyDriverHandle, IOCTL_UCMPROXY_TYPEC_POWER_DIRECTION_CHANGE,
        (LPVOID)&Request, sizeof(PROXY_ROLE_CHANGE_REQUEST),
        NULL, 0,
        NULL, NULL
    );

    if (!ret) {
        printf("[ERROR] Proxy component reports failure, error %d\n", GetLastError());
        return STATUS_UNSUCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS DlWdfCxUcmFunctionImplStub()
{
    // Calling a function that is not yet implemented.
    printf("[ERROR] Calling a unimplemented WDF UcmCx stub function\n");

    if (IsDebuggerPresent())
    {
        DebugBreak();
    }

    return STATUS_NOT_IMPLEMENTED;
}
